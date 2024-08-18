/*******************************************************************************
 *  @file: lua_thread.h
 *   
 *  @brief: Header for lua_thread script runner.
*******************************************************************************/
#ifndef LUA_THREAD_H
#define LUA_THREAD_H

#include <stdint.h>
#include "RtosUtils.h"
#include "Fs_Api.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LUA_THREAD_MAX_FILENAME     64

typedef struct Lua_Thread_sQueueItem
{
    char script_file[LUA_THREAD_MAX_FILENAME];
} Lua_Thread_sQueueItem;

/** @brief Defines an LED strip.
*/
typedef struct Lua_Thread
{
    /** @brief Pointer to file system api. */
    Fs_Api *fs;

    /** @brief Task stack size. */
    uint16_t taskStackSize;
    /** @brief Task name. */
    char taskName[16];
    /** @brief Task prio. */
    uint8_t taskPrio;

    /* Internal Members. */
    /** @brief Send Queue */
    RTOS_QUEUE squeue;

    /** @brief Task handle. */
    RTOS_TASK taskHandle;
    
} Lua_Thread;

/******************************************************************************
    [docexport Lua_Thread_send]
*//**
    @brief Sends a script to be executed to the queue.
    @param[in] self to Lua_Thread object.
    @param[in] filename  Filename of script.
******************************************************************************/
int
Lua_Thread_send(Lua_Thread *self, const char *filename);

/******************************************************************************
    [docexport Lua_Thread_init]
*//**
    @brief Initializes a thread for executing Lua scripts.
    @param[in] t  Pointer to the Lua_Thread object.
******************************************************************************/
esp_err_t
Lua_Thread_init(Lua_Thread *t);
#endif
