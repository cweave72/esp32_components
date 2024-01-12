/*******************************************************************************
 *  @file: ProtoRpc.h
 *   
 *  @brief: Header for ProtoRpc.
*******************************************************************************/
#ifndef PROTORPC_H
#define PROTORPC_H
#include <stddef.h>
#include <stdint.h>
#include "ProtoRpc.pb.h"

typedef void ProtoRpc_handler(void *call_frame, void *reply_frame, StatusEnum *status);
typedef ProtoRpc_handler * ProtoRpc_resolver(void *call_frame, uint32_t offset);

typedef struct ProtoRpc_Resolver_Entry
{
    /** @brief The callset tag in the RpcFrame. */
    uint32_t tag;
    /** @brief Pointer to the resolver function. */
    ProtoRpc_resolver *resolver;

} ProtoRpc_Resolver_Entry;

typedef ProtoRpc_Resolver_Entry * ProtoRpc_resolvers;

#define PROTORPC_ADD_CALLSET(callset_tag, callset_resolver) \
{ .tag = (callset_tag), .resolver = (callset_resolver) }

typedef struct ProtoRpc_info
{
    size_t header_offset;
    size_t which_callset_offset;
    size_t callset_offset;
    const void *frame_fields;
    ///** @brief Array of Resolver entry pointers. */
    //ProtoRpc_Resolver_Entry **resolver_entries;
} ProtoRpc_info;

typedef struct ProtoRpc_Handler_Entry
{
    /** @brief The handler tag in the callset. */
    uint32_t tag;
    /** @brief Pointer to the handler function. */
    ProtoRpc_handler *handler;

} ProtoRpc_Handler_Entry;

typedef ProtoRpc_Handler_Entry *ProtoRpc_handlers;

#define PROTORPC_ADD_HANDLER(handler_tag, handler_func)\
{ .tag = (handler_tag), .handler = (handler_func) }

#define PROTORPC_ARRAY_LENGTH(array)\
    (sizeof((array)) / sizeof((array)[0]))

/******************************************************************************
    [docexport ProtoRpc_server]
*//**
    @brief Decoded received ProtoRpc frame, executes the RPC, provides the reply.
******************************************************************************/
void
ProtoRpc_server(
    ProtoRpc_info *info,
    ProtoRpc_resolvers resolvers,
    uint32_t num_resolvers,
    uint8_t *rcvd_buf,
    uint32_t rcvd_buf_size,
    uint8_t *reply_buf,
    uint32_t reply_buf_max_size,
    uint32_t *reply_encoded_size);
#endif
