/*******************************************************************************
 *  @file: lua_thread_rpc.h
 *
 *  @brief: Header for lua_thread_rpc.
*******************************************************************************/
#ifndef LUA_THREAD_RPC_H
#define LUA_THREAD_RPC_H

#include <stdint.h>
#include "ProtoRpc.h"
#include "lua_thread.h"

/******************************************************************************
    [docexport Lua_Thread_init]
*//**
    @brief Initializes a thread for executing Lua scripts.
    @param[in] t  Pointer to the Lua_Thread object.
******************************************************************************/
void
lua_thread_rpc_init(Lua_Thread *t);

/******************************************************************************
    [docexport lua_thread_rpc_resolver]
*//**
    @brief Resolver function for lua_thread_rpc.
******************************************************************************/
ProtoRpc_handler *
lua_thread_rpc_resolver(void *call_frame, uint32_t offset);
#endif
