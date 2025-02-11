#ifndef __UTILS_H__
#define __UTILS_H__

#include "esp_log.h"

#define DEBUG_ACTIVE

#ifdef DEBUG_ACTIVE
  #define LOG_PROLOG ESP_LOGI(MODULE_NAME, "Started at [%s]", __func__);
  #define LOG_EPILOG ESP_LOGI(MODULE_NAME, "Finished at [%s]", __func__);
#else
  #define LOG_PROLOG
  #define LOG_EPILOG
#endif

#endif /** !__UTILS_H__ */
