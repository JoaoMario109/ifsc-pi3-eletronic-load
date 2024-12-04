#include "common.h"
#include "bus/i2c.h"

#include "driver/gpio.h"
#include "esp_log.h"

/** Handlers */
i2c_master_bus_handle_t h_i2c0;
i2c_master_dev_handle_t h_i2c0_load;

/** Forward Decl */
static void i2c_add_load_module(void);

/**
 * @brief Init default board SPI bus using maximum transfer size needed by LVGL
 * @return void
 */
void i2c_init(void)
{
    i2c_master_bus_config_t i2c_mst_config = {0};
    i2c_mst_config.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_mst_config.i2c_port = I2C_NUM_0;
    i2c_mst_config.scl_io_num = GPIO_I2C_SCL;
    i2c_mst_config.sda_io_num = GPIO_I2C_SDA;
    i2c_mst_config.glitch_ignore_cnt = 7;
    i2c_mst_config.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &h_i2c0));

    i2c_add_load_module();
}

/** Implementations */

static void i2c_add_load_module(void)
{
    i2c_device_config_t dev_cfg = {0};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = LOAD_MODULE_SLAVE_ADDR;
    dev_cfg.scl_speed_hz = I2C_CLOCK_SPEED;
    dev_cfg.flags.disable_ack_check = 0;

    ESP_ERROR_CHECK(i2c_master_bus_add_device(h_i2c0, &dev_cfg, &h_i2c0_load));
}

void i2c_send_bytes_to_load(void* bytes, uint16_t size)
{
    i2c_master_bus_reset(h_i2c0);
    i2c_master_transmit(h_i2c0_load, bytes, size, -1);
}
