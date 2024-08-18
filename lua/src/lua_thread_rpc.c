/*******************************************************************************
 *  @file: lua_thread_rpc.c
 *
 *  @brief: Handlers for lua_thread_rpc.
*******************************************************************************/
#include "lua_thread_rpc.h"
#include "LogPrint.h"
#include "LogPrint_local.h"
#include "ProtoRpc.pb.h"
#include "lua_thread.h"
#include "lua_thread_rpc.pb.h"

static const char *TAG = "lua_thread_rpc";

static Lua_Thread *luathread = NULL;

/******************************************************************************
    runScript

    Call params:
        call->filename: string 
    Reply params:
        reply->status: int32 
*//**
    @brief Implements the RPC runScript handler.
******************************************************************************/
static void
runScript(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lua_LuaCallset *call_msg = (lua_LuaCallset *)call_frame;
    lua_LuaCallset *reply_msg = (lua_LuaCallset *)reply_frame;
    lua_RunScript_call *call = &call_msg->msg.runScript_call;
    lua_RunScript_reply *reply = &reply_msg->msg.runScript_reply;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("In runScript handler");

    reply_msg->which_msg = lua_LuaCallset_runScript_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    reply->status = Lua_Thread_send(luathread, call->filename);
}

/******************************************************************************
    getLastMessage

    Call params:
    Reply params:
        reply->msg: string 
*//**
    @brief Implements the RPC getLastMessage handler.
******************************************************************************/
static void
getLastMessage(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lua_LuaCallset *call_msg = (lua_LuaCallset *)call_frame;
    lua_LuaCallset *reply_msg = (lua_LuaCallset *)reply_frame;
    lua_GetLastMessage_call *call = &call_msg->msg.getLastMessage_call;
    lua_GetLastMessage_reply *reply = &reply_msg->msg.getLastMessage_reply;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("In getLastMessage handler");

    reply_msg->which_msg = lua_LuaCallset_getLastMessage_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    /* TODO: Implement handler */
}



static ProtoRpc_Handler_Entry handlers[] = {
    PROTORPC_ADD_HANDLER(lua_LuaCallset_runScript_call_tag, runScript),
    PROTORPC_ADD_HANDLER(lua_LuaCallset_getLastMessage_call_tag, getLastMessage),
};

#define NUM_HANDLERS    PROTORPC_ARRAY_LENGTH(handlers)

/******************************************************************************
    [docimport Lua_Thread_init]
*//**
    @brief Initializes a thread for executing Lua scripts.
    @param[in] t  Pointer to the Lua_Thread object.
******************************************************************************/
void
lua_thread_rpc_init(Lua_Thread *t)
{
    luathread = t;
}

/******************************************************************************
    [docimport lua_thread_rpc_resolver]
*//**
    @brief Resolver function for lua_thread_rpc.
    @param[in] call_frame  Pointer to the unpacked call frame object.
    @param[in] offset  Offset of the callset member within the call_frame.
******************************************************************************/
ProtoRpc_handler *
lua_thread_rpc_resolver(void *call_frame, uint32_t offset)
{
    uint8_t *frame = (uint8_t *)call_frame;
    lua_LuaCallset *this = (lua_LuaCallset *)&frame[offset];
    unsigned int i;

    /** @brief Handler lookup */
    for (i = 0; i < NUM_HANDLERS; i++)
    {
        ProtoRpc_Handler_Entry *entry = &handlers[i];
        if (entry->tag == this->which_msg)
        {
            return entry->handler;
        }
    }

    return NULL;
}
