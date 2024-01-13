/*******************************************************************************
 *  @file: RtosUtils.h
 *   
 *  @brief: Header for RtosUtils.
*******************************************************************************/
#ifndef RTOSUTILS_H
#define RTOSUTILS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "LogPrint.h"

/** @brief Task creation.  Returns 0 on success, -1 on error. */
#define RTOS_TASK_CREATE(func, name, stack, params, prio, handle)             \
({                                                                            \
    BaseType_t xret;                                                          \
    int ret = 0;                                                              \
    xret = xTaskCreate((func), (name), (stack), (params), (prio), (handle));  \
    if (xret != pdPASS)                                                       \
    {                                                                         \
        LOGPRINT_ERROR("Error creating task.");                               \
        ret = -1;                                                             \
    }                                                                         \
    ret;                                                                      \
})

#define RTOS_MS_TO_TICKS(ms)        ((ms)/portTICK_PERIOD_MS)
#define RTOS_SEC_TO_TICKS(s)        ((s)*1000/portTICK_PERIOD_MS)

/** @brief Macro wrapper for task sleep. */
#define RTOS_TASK_SLEEP_ms(ms)      vTaskDelay(RTOS_MS_TO_TICKS(ms))
#define RTOS_TASK_SLEEP_s(s)        vTaskDelay(RTOS_SEC_TO_TICKS(s))

/** @brief Event Flag Macros */
#define RTOS_FLAGS                      EventBits_t
#define RTOS_FLAG_GROUP                 EventGroupHandle_t
#define RTOS_FLAG_GROUP_CREATE()        xEventGroupCreate()

/*  Wait on all flags forever, self-clear
    This macro will only return if all event flags are set.
*/
#define RTOS_PEND_ALL_FLAGS_CLR(grp, eflags)                                  \
({                                                                            \
  EventBits_t flags;                                                          \
  flags = xEventGroupWaitBits((grp), (eflags), pdTRUE, pdTRUE, portMAX_DELAY);\
  flags;                                                                      \
})

/*  Wait on all flags for x ms, self-clear.
    This macro will return when either all the flags are set or a timeout occurred.
    Test flags on return. If (flags & eflags) != eflags, timeout occurred.
*/
#define RTOS_PEND_ALL_FLAGS_CLR_MS(grp, eflags, ms)   \
({                                                    \
  EventBits_t flags;                                  \
  flags = xEventGroupWaitBits((grp),                  \
                              (eflags),               \
                              pdTRUE,                 \
                              pdTRUE,                 \
                              RTOS_MS_TO_TICKS(ms));  \
  flags;                                              \
})

/*  Wait on any flags forever, self-clear
    This macro will return if any event flags are set. Test returned flags to
    determine which one was set.
*/
#define RTOS_PEND_ANY_FLAGS_CLR(grp, eflags)                                   \
({                                                                             \
  EventBits_t flags;                                                           \
  flags = xEventGroupWaitBits((grp), (eflags), pdTRUE, pdFALSE, portMAX_DELAY);\
  flags;                                                                       \
})

/*  Wait on any flags for x ms, self-clear.
    This macro will return when either any the flags are set or a timeout occurred.
    Test flags on return to determine if any were set. If none are set, a
    timeout occurred.
*/
#define RTOS_PEND_ANY_FLAGS_CLR_MS(grp, eflags, ms)   \
({                                                    \
  EventBits_t flags;                                  \
  flags = xEventGroupWaitBits((grp),                  \
                              (eflags),               \
                              pdTRUE,                 \
                              pdFALSE,                \
                              RTOS_MS_TO_TICKS(ms));  \
  flags;                                              \
})

/* Set Flags */
#define RTOS_SET_FLAGS(grp, eflags)     xEventGroupSetBits((grp), (eflags))

/* Clear Flags */
#define RTOS_CLR_FLAGS(grp, eflags)     xEventGroupClearBits((grp), (eflags))

/* Get Current Flags. */
#define RTOS_GET_FLAGS(grp)             xEventGroupGetBits((grp))

#endif
