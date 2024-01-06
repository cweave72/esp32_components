/*******************************************************************************
 *  @file: LogPrint.h
 *   
 *  @brief: Macros for ESP32 logging.
*******************************************************************************/
#ifndef LOGPRINT_H
#define LOGPRINT_H

#include "esp_log.h"

#define LOGPRINT_ERROR(fmt, ...) \
    ESP_LOGE(TAG, "(l:%u) " fmt, __LINE__, ##__VA_ARGS__)

#define LOGPRINT_INFO(fmt, ...) \
    ESP_LOGI(TAG, "(l:%u) " fmt, __LINE__, ##__VA_ARGS__)

#define LOGPRINT_DEBUG(fmt, ...) \
    ESP_LOGD(TAG, "(l:%u) " fmt, __LINE__, ##__VA_ARGS__)

#define LOGPRINT_HEXDUMP_ERROR(msg, buf, size) \
    ESP_LOGE(TAG, "(l:%u) " msg, __LINE__); \
    ESP_LOG_BUFFER_HEXDUMP(TAG, (buf), (size), ESP_LOG_ERROR)

#define LOGPRINT_HEXDUMP_INFO(msg, buf, size) \
    ESP_LOGI(TAG, "(l:%u) " msg, __LINE__); \
    ESP_LOG_BUFFER_HEXDUMP(TAG, (buf), (size), ESP_LOG_INFO)

#define LOGPRINT_HEXDUMP_DEBUG(msg, buf, size) \
    ESP_LOGD(TAG, "(l:%u) " msg, __LINE__); \
    ESP_LOG_BUFFER_HEXDUMP(TAG, (buf), (size), ESP_LOG_DEBUG)

#endif
