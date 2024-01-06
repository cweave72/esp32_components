/*******************************************************************************
 *  @file: TestRpc.c
 *  
 *  @brief: Handlers for TestRpc.
*******************************************************************************/
#include "ProtoRpc.h"
#include "ProtoRpc.pb.h"
#include "TestRpc.pb.h"
#include "LogPrint.h"

static const char *TAG = "TestRpc";

/******************************************************************************
    add
*//**
    @brief Implements the RPC add handler.
******************************************************************************/
static void
add(void *call_frame, void *reply_frame, StatusEnum *status)
{
    test_TestCallset *call_msg = (test_TestCallset *)call_frame;
    test_TestCallset *reply_msg = (test_TestCallset *)reply_frame;
    test_Add_call *call = &call_msg->msg.add_call;
    test_Add_reply *reply = &reply_msg->msg.add_reply;

    LOGPRINT_INFO("In add handler: a = %d; b = %d", (int)call->a, (int)call->b);

    reply_msg->which_msg = test_TestCallset_add_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    reply->sum = call->a + call->b;
}

/******************************************************************************
    bad_handler
*//**
    @brief Implements the RPC handler_error.
******************************************************************************/
static void
handler_error(void *call_frame, void *reply_frame, StatusEnum *status)
{
    test_TestCallset *call_msg = (test_TestCallset *)call_frame;
    test_TestCallset *reply_msg = (test_TestCallset *)reply_frame;
    test_HandlerError_call *call = &call_msg->msg.handlererror_call;
    test_HandlerError_reply *reply = &reply_msg->msg.handlererror_reply;

    (void)call;
    (void)reply;

    LOGPRINT_INFO("In handler_error handler");
    reply_msg->which_msg = test_TestCallset_handlererror_reply_tag;
    *status = StatusEnum_RPC_HANDLER_ERROR;
}

/******************************************************************************
    setstruct
*//**
    @brief Implements the RPC setstruct handler.
******************************************************************************/
static void
setstruct(void *call_frame, void *reply_frame, StatusEnum *status)
{
    test_TestCallset *call_msg = (test_TestCallset *)call_frame;
    test_TestCallset *reply_msg = (test_TestCallset *)reply_frame;
    test_SetStruct_call *call = &call_msg->msg.setstruct_call;
    test_SetStruct_reply *reply = &reply_msg->msg.setstruct_reply;

    (void)reply;

    LOGPRINT_INFO("In setstruct handler:");
    reply_msg->which_msg = test_TestCallset_setstruct_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;


    LOGPRINT_INFO("   var_int32 = %d", (int)call->var_int32);
    LOGPRINT_INFO("   var_uint32 = %u", (unsigned int)call->var_uint32);
    LOGPRINT_INFO("   var_int64 = 0x%08x%08x",
        (unsigned int)(call->var_int64 >> 32),
        (unsigned int)call->var_int64);
    LOGPRINT_INFO("   var_uint64 = 0x%08x%08x",
        (unsigned int)(call->var_uint64 >> 32),
        (unsigned int)call->var_uint64);
    for (int i = 0; i < call->var_uint32_array_count; i++)
    {
        LOGPRINT_INFO("   var_uint32[%u] = %u", i, (unsigned int)call->var_uint32_array[i]);
    }
    LOGPRINT_INFO("   var_bool = %s", (call->var_bool) ? "true" : "false");
    LOGPRINT_INFO("   var_string = %s", call->var_string);
    LOGPRINT_INFO("   var_bytes (%u):", (unsigned int)call->var_bytes.size);
    ESP_LOG_BUFFER_HEXDUMP(TAG, &call->var_bytes.bytes, call->var_bytes.size, ESP_LOG_INFO);
}

static ProtoRpc_Handler_Entry handlers[] = {
    PROTORPC_ADD_HANDLER(test_TestCallset_add_call_tag, add),
    PROTORPC_ADD_HANDLER(test_TestCallset_setstruct_call_tag, setstruct),
    PROTORPC_ADD_HANDLER(test_TestCallset_handlererror_call_tag, handler_error)
};

#define NUM_HANDLERS    PROTORPC_ARRAY_LENGTH(handlers)

/******************************************************************************
    [docimport TestRpc_resolver]
*//**
    @brief Resolver function for TestRpc.
******************************************************************************/
ProtoRpc_handler *
TestRpc_resolver(void *call_frame, uint32_t offset)
{
    uint8_t *frame = (uint8_t *)call_frame;
    test_TestCallset *this = (test_TestCallset *)&frame[offset];
    unsigned int i;

    LOGPRINT_DEBUG("which_msg = %u", (unsigned int)this->which_msg);

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
