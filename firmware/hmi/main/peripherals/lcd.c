#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_st7735.h"

#include "bus/spi.h"
#include "common.h"
#include "peripherals/lcd.h"
#include "utils.h"

/** Definitions */

#define MODULE_NAME "peripherals.lcd"

/** Handlers */

TaskHandle_t h_lvgl_task;
SemaphoreHandle_t h_lvgl_mutex;

lv_disp_draw_buf_t h_disp_buf;
lv_disp_drv_t h_disp_drv;
lv_disp_t *h_disp;

esp_timer_handle_t h_lvgl_tick_timer;
esp_lcd_panel_io_handle_t h_io_handle;
esp_lcd_panel_handle_t h_panel_handle;

/** Forward Decl */

static void lcd_init_spi(void);
static void lcd_init_panel(void);
static void lcd_timer_init(void);

static void lvgl_init(void);
static void lvgl_increase_tick(void *arg);
static void lvgl_task_impl(void *arg);

static bool on_lvgl_flush_ready(
  esp_lcd_panel_io_handle_t panel_io,
  esp_lcd_panel_io_event_data_t *edata,
  void *user_ctx
);
static void on_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
static void on_lvgl_port_update_callback(lv_disp_drv_t *drv);

/**
 * @brief Set up the LCD basic configuration
 */
void lcd_init(void)
{
  LOG_PROLOG

  spi_mutex_lock(-1);

  lcd_init_spi();
  lcd_init_panel();
  lvgl_init();

  spi_mutex_unlock();

  LOG_EPILOG
}

/**
 * @brief Start LVGL task with a given UI
 * @return void
 */
void lcd_start(ui_fn_t ui_fn)
{
  xTaskCreate(
    lvgl_task_impl, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, &h_lvgl_task
  );
  configASSERT(h_lvgl_task);

  if (lvgl_mutex_lock(-1)) {
    ui_fn(h_disp);
    lvgl_mutex_unlock();
  }
}

/**
 * @brief Stop and detroy LVGL task
 * @return void
 */
void lcd_stop(void)
{
  vTaskDelete(h_lvgl_task);
  h_lvgl_task = NULL;
}

/**
 * @brief Locks the LVGL mutex due to the LVGL APIs are not thread-safe
 * @param timeout_ms Timeout in milliseconds to wait for the mutex, if -1, wait indefinitely
 * @return bool
 */
bool lvgl_mutex_lock(int timeout_ms)
{
  const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTakeRecursive(h_lvgl_mutex, timeout_ticks) == pdTRUE;
}

/**
 * @brief Unlocks the LVGL mutex
 * @return void
 */
void lvgl_mutex_unlock(void)
{
  xSemaphoreGiveRecursive(h_lvgl_mutex);
}

/** Implementations */

static void lcd_init_spi(void)
{
  LOG_PROLOG

  esp_lcd_panel_io_spi_config_t io_config = {
    .dc_gpio_num = GPIO_LCD_DC,
    .cs_gpio_num = GPIO_LCD_CS,
    .pclk_hz = LCD_PIXEL_CLOCK_HZ,
    .lcd_cmd_bits = LCD_CMD_BITS,
    .lcd_param_bits = LCD_PARAM_BITS,
    .spi_mode = 0,
    .trans_queue_depth = 10,
    .on_color_trans_done = on_lvgl_flush_ready,
    .user_ctx = &h_disp_drv,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &h_io_handle));

  LOG_EPILOG
}

void lcd_init_panel(void)
{
  LOG_PROLOG

  esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = GPIO_LCD_RST,
    .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
    .bits_per_pixel = 16,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7735(h_io_handle, &panel_config, &h_panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(h_panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(h_panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(h_panel_handle, true));
  ESP_ERROR_CHECK(esp_lcd_panel_mirror(h_panel_handle, false, true));
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(h_panel_handle, true));

  LOG_EPILOG
}

void lvgl_init(void)
{
  LOG_PROLOG

  lv_init();
  lv_color_t *buf1 = heap_caps_malloc(LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf1);
  lv_color_t *buf2 = heap_caps_malloc(LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf2);

  lv_disp_draw_buf_init(&h_disp_buf, buf1, buf2, LCD_H_RES * 20);
  lv_disp_drv_init(&h_disp_drv);
  h_disp_drv.hor_res = LCD_H_RES;
  h_disp_drv.ver_res = LCD_V_RES;
  h_disp_drv.flush_cb = on_lvgl_flush_cb;
  h_disp_drv.drv_update_cb = on_lvgl_port_update_callback;
  h_disp_drv.draw_buf = &h_disp_buf;
  h_disp_drv.user_data = h_panel_handle;
  h_disp = lv_disp_drv_register(&h_disp_drv);

  lcd_timer_init();

  h_lvgl_mutex = xSemaphoreCreateRecursiveMutex();
  assert(h_lvgl_mutex);

  LOG_EPILOG
}

void lcd_timer_init(void)
{
  LOG_PROLOG

  const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &lvgl_increase_tick,
    .name = "lvgl_tick"
  };
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &h_lvgl_tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(h_lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

  LOG_EPILOG
}

/** Interrupts */
static bool on_lvgl_flush_ready(
  esp_lcd_panel_io_handle_t panel_io,
  esp_lcd_panel_io_event_data_t *edata,
  void *user_ctx
)
{
  lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
  lv_disp_flush_ready(disp_driver);
  return false;
}

static void on_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
  spi_mutex_lock(-1);

  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
  int offsetx1 = area->x1;
  int offsetx2 = area->x2;
  int offsety1 = area->y1;
  int offsety2 = area->y2;
  esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);

  spi_mutex_unlock();
}

static void on_lvgl_port_update_callback(lv_disp_drv_t *drv)
{
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;

  switch (drv->rotated)
  {
  case LV_DISP_ROT_NONE:
    // Rotate LCD display
    esp_lcd_panel_swap_xy(h_panel_handle, true);
    esp_lcd_panel_mirror(h_panel_handle, false, true);
    break;
  case LV_DISP_ROT_90:
    // Rotate LCD display
    esp_lcd_panel_swap_xy(panel_handle, false);
    esp_lcd_panel_mirror(panel_handle, false, false);
    break;
  case LV_DISP_ROT_180:
    // Rotate LCD display
    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, true, true);
    break;
  case LV_DISP_ROT_270:
    // Rotate LCD display
    esp_lcd_panel_swap_xy(panel_handle, false);
    esp_lcd_panel_mirror(panel_handle, true, false);
    break;
  }
}

static void lvgl_increase_tick(void *arg)
{
  lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

static void lvgl_task_impl(void *arg)
{
  uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
  while (1) {
    if (lvgl_mutex_lock(-1)) {
      task_delay_ms = lv_timer_handler();
      lvgl_mutex_unlock();
    }
    if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS) {
      task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
    } else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS) {
      task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
    }
    vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
  }
}
