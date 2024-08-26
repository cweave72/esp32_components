/*******************************************************************************
 *  @file: SwTimer.h
 *   
 *  @brief: Header for SwTimer
*******************************************************************************/
#ifndef SWTIMER_H
#define SWTIMER_H

#include <stdint.h>
#include <stdbool.h>
#include <esp_timer.h>

typedef void SwTimer_cb(void *arg);

typedef enum SwTimerType
{
    SWTIMER_TYPE_ONE_SHOT = 0,
    SWTIMER_TYPE_PERIODIC
} SwTimerType;

typedef enum SwTimerState
{
    STATE_IDLE = 0,
    STATE_RUNNING,
    STATE_EXPIRED
    
} SwTimerState;

typedef struct SwTimer
{
    /** @brief Internal state variables. */
    SwTimerState state;
    uint64_t capture;
    uint64_t delay_us;
    /** @brief One-shot and periodic args */
    SwTimer_cb *cb;
    void *cb_arg;
    const char *name;
    /* internal */
    esp_timer_handle_t handle;
    SwTimerType type;
} SwTimer;

/** @brief Millisecond version of functions. */
#define SwTimer_setMs(swt, ms)      SwTimer_setUs((swt), (ms)*1000)
#define SwTimer_sleepMs(ms)         SwTimer_sleepUs((ms)*1000)

/******************************************************************************
    [docexport SwTimer_getCount]
*//**
    @brief Gets the current timer value.
    @return Returns the current count
******************************************************************************/
uint64_t
SwTimer_getCount(void);

/******************************************************************************
    [docexport SwTimer_tic]
*//**
    @brief Start a elapsed time measurement.
    @param[in] swt  Pointer to a SwTimer object.
    @return Returns the current count.
******************************************************************************/
uint64_t
SwTimer_tic(SwTimer *swt);

/******************************************************************************
    [docexport SwTimer_toc]
*//**
    @brief Finishes a elapsed time measurement.
    @param[in] swt  Pointer to a SwTimer object.
    @return Returns the delta, in microseconds.
******************************************************************************/
uint64_t
SwTimer_toc(SwTimer *swt);

/******************************************************************************
    [docexport SwTimer_setUs]
*//**
    @brief Initializes a timer test duration, us.
    @param[in] swt  Pointer to a SwTimer object.
    @param[in] us  Microseconds.
******************************************************************************/
void
SwTimer_setUs(SwTimer *swt, uint64_t us);

/******************************************************************************
    [docexport SwTimer_sleepUs]
*//**
    @brief Sleeps for provided us.
    @param[in] us  Microseconds.
******************************************************************************/
void
SwTimer_sleepUs(uint64_t us);

/******************************************************************************
    [docexport SwTimer_test]
*//**
    @brief Tests a SwTimer object for elapsed time.
    Call SwTimer_setUs first.
    @param[in] swt  Pointer to a SwTimer object.
    @return Returns true if the timer has elapsed.
******************************************************************************/
bool
SwTimer_test(SwTimer *swt);

/******************************************************************************
    [docexport SwTimer_create]
*//**
    @brief Creates a timer with callback.
******************************************************************************/
int
SwTimer_create(
    SwTimer *swt,
    void *cb,
    void *cb_arg);

/******************************************************************************
    [docexport SwTimer_start]
*//**
    @brief Starts a previously created SwTimer object.
******************************************************************************/
int
SwTimer_start(SwTimer *swt, SwTimerType type, uint64_t duration_us);

/******************************************************************************
    [docexport SwTimer_stop]
*//**
    @brief Stops a previously started SwTimer object.
******************************************************************************/
int
SwTimer_stop(SwTimer *swt);
#endif
