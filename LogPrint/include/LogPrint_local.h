/*******************************************************************************
 *  @file: LogPrint_local.h
 *   
 *  @brief: Header which allows component scope logging level.
 *  This is somewhat of a trick to achieve component-level log level control.
 *
 *  How to use:
 *  1. #include "LogPrint_local.h"  in your source file
 *  2. Add the following to the component CMakeLists.txt file:
 *
 *   set(local_log_level "LOCAL_DEBUG")
 *   
 *   target_compile_definitions(
 *       ${COMPONENT_LIB}
 *       PRIVATE
 *       "-D${local_log_level}"
 *       )
 *
 *  3. Add the following to the applications app_main():
 *     esp_log_level_set("*", ESP_LOG_DEBUG);
*******************************************************************************/
#if defined(LOCAL_VERBOSE)
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL     ESP_LOG_VERBOSE
#include "esp_log.h"

#elif defined(LOCAL_DEBUG)
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL     ESP_LOG_DEBUG
#include "esp_log.h"

#elif defined(LOCAL_INFO)
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL     ESP_LOG_INFO
#include "esp_log.h"

#endif
