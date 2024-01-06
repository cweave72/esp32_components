#include <stdio.h>
#include "esp_err.h"
#include "SwTimer.h"
#include "LogPrint.h"

const char *TAG = "SwTimer";

/** @brief Macro which calculates the circular delta from a reference count to
      the current count.  Since counts are 64-bit, no worry about wrap.
  */
#define DELTA(cnt, ref)     ((cnt) - (ref))

#define RETURN_ON_ERR(ret, msg)                                   \
do {                                                              \
    if ((ret) != ESP_OK) {                                        \
        LOGPRINT_ERROR("%s : 0x%08x", msg, ret);                  \
        return -1;                                                \
    }                                                             \
} while (0)


/******************************************************************************
    [docimport SwTimer_getCount]
*//**
    @brief Gets the current timer value.
    @return Returns the current count
******************************************************************************/
uint64_t
SwTimer_getCount(void)
{
    return esp_timer_get_time();
}

/******************************************************************************
    [docimport SwTimer_tic]
*//**
    @brief Start a elapsed time measurement.
    @param[in] swt  Pointer to a SwTimer object.
    @return Returns the current count.
******************************************************************************/
uint64_t
SwTimer_tic(SwTimer *swt)
{
    uint32_t capture = esp_timer_get_time();
    swt->capture = capture;
    return capture;
}

/******************************************************************************
    [docimport SwTimer_toc]
*//**
    @brief Finishes a elapsed time measurement.
    @param[in] swt  Pointer to a SwTimer object.
    @return Returns the delta, in microseconds.
******************************************************************************/
uint64_t
SwTimer_toc(SwTimer *swt)
{
    uint64_t now = esp_timer_get_time();
    return DELTA(now, swt->capture);
}

/******************************************************************************
    [docimport SwTimer_setUs]
*//**
    @brief Initializes a timer test duration, us.
    @param[in] swt  Pointer to a SwTimer object.
    @param[in] us  Microseconds.
******************************************************************************/
void
SwTimer_setUs(SwTimer *swt, uint64_t us)
{
    swt->capture = esp_timer_get_time();
    swt->delay_us = us;
    swt->state = STATE_RUNNING;
}

/******************************************************************************
    [docimport SwTimer_test]
*//**
    @brief Tests a SwTimer object for elapsed time.
    Call SwTimer_setUs first.
    @param[in] swt  Pointer to a SwTimer object.
    @return Returns true if the timer has elapsed.
******************************************************************************/
bool
SwTimer_test(SwTimer *swt)
{
    uint64_t now = esp_timer_get_time();

    if (swt->state == STATE_EXPIRED)
    {
        return true;
    }
    else if (swt->state != STATE_RUNNING)
    {
        LOGPRINT_ERROR("SwTimer not running, call SwTimer_setUs first.\r\n");
        return true;
    }

    if (DELTA(now, swt->capture) >= swt->delay_us)
    {
        swt->state = STATE_EXPIRED;
        return true;
    }

    return false;
}

/******************************************************************************
    [docimport SwTimer_create]
*//**
    @brief Creates a timer with callback.
******************************************************************************/
int
SwTimer_create(
    SwTimer *swt,
    void *cb,
    void *cb_arg)
{
    esp_err_t ret;

    const esp_timer_create_args_t create_args = {
        .callback = (esp_timer_cb_t)cb,
        .arg = cb_arg,
        .name = "SwTimer",
    };

    /* Create the timer, but it is not running yet. */
    ret = esp_timer_create(&create_args, &swt->handle);
    RETURN_ON_ERR(ret, "Error creating timer");
    
    return 0;
}

/******************************************************************************
    [docimport SwTimer_start]
*//**
    @brief Starts a previously created SwTimer object.
******************************************************************************/
int
SwTimer_start(SwTimer *swt, SwTimerType type, uint64_t duration_us)
{
    esp_err_t ret;
    swt->type = type;

    if (type == SWTIMER_TYPE_ONE_SHOT)
    {
        ret = esp_timer_start_once(swt->handle, duration_us);
        RETURN_ON_ERR(ret, "Error starting one-shot timer");
    }
    else
    {
        ret = esp_timer_start_periodic(swt->handle, duration_us);
        RETURN_ON_ERR(ret, "Error starting periodic timer");
    }

    return 0;
}

/******************************************************************************
    [docimport SwTimer_stop]
*//**
    @brief Stops a previously started SwTimer object.
******************************************************************************/
int
SwTimer_stop(SwTimer *swt)
{
    esp_err_t ret;
    ret = esp_timer_stop(swt->handle);
    RETURN_ON_ERR(ret, "Error stopping timer");
    return 0;
}
