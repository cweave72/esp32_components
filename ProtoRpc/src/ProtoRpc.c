/*******************************************************************************
 *  @file: ProtoRpc.c
 *  
 *  @brief: Implements a Protobuf-based RPC server.
*******************************************************************************/
#include "LogPrint.h"
#include "PbGeneric.h"
#include "ProtoRpc.pb.h"
#include "ProtoRpc.h"

/* Required for LOGPRINTs */
static const char *TAG = "ProtoRpc";

/** @brief Static frame object to decode into. */
static uint8_t rpc_call_frame[PROTORPC_MSG_MAX_SIZE] = { 0 };
static uint8_t rpc_reply_frame[PROTORPC_MSG_MAX_SIZE] = { 0 };

/******************************************************************************
    callset_lookup
*//**
    @brief Performs a callset lookup based on the which_callset tag.
******************************************************************************/
static ProtoRpc_resolver *
callset_lookup(
    uint32_t which_callset,
    ProtoRpc_Resolver_Entry *resolvers,
    uint32_t num_callsets)
{
    ProtoRpc_Resolver_Entry *entry;
    uint32_t i;

    for (i = 0; i < num_callsets; i++)
    {
        entry = resolvers + i;
        if (entry->tag == which_callset)
        {
            return entry->resolver;
        }
    }

    return NULL;
}

/******************************************************************************
    [docimport ProtoRpc_server]
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
    uint32_t *reply_encoded_size)
{
    uint32_t ret;
    ProtoRpc_resolver *resolver;
    ProtoRpcHeader *header;
    ProtoRpcHeader *reply_header;
    ProtoRpc_handler *handler;
    size_t which_callset;

    *reply_encoded_size = 0;

    /* Unpack the received buffer into rpc_frame. */
    ret = Pb_unpack(rcvd_buf, rcvd_buf_size, rpc_call_frame, info->frame_fields);
    if (!ret)
    {
        LOGPRINT_HEXDUMP_ERROR("Pb_unpack_failed", rcvd_buf, rcvd_buf_size);
        return;
    }

    header = (ProtoRpcHeader *)&rpc_call_frame[info->header_offset];
    which_callset = rpc_call_frame[info->which_callset_offset];

    reply_header = (ProtoRpcHeader *)&rpc_reply_frame[info->header_offset];

    LOGPRINT_INFO("seqn = %u; no_reply = %u; which_callset = %u",
        (unsigned int)header->seqn,
        (unsigned int)header->no_reply,
        (unsigned int)which_callset);

    /** @brief Get the callset resolver. */
    resolver = callset_lookup(which_callset, resolvers, num_resolvers);
    if (!resolver)
    {
        LOGPRINT_ERROR("Bad resolver lookup (which_callset=%u).",
            (unsigned int)which_callset);
        reply_header->seqn = header->seqn;
        reply_header->status = StatusEnum_RPC_BAD_RESOLVER_LOOKUP;
        *reply_encoded_size = Pb_pack(reply_buf,
                                      reply_buf_max_size,
                                      &rpc_reply_frame,
                                      info->frame_fields);
        return;
    }
    LOGPRINT_DEBUG("Got resolver 0x%08x", (unsigned int)resolver);

    handler = resolver(rpc_call_frame, info->callset_offset);
    if (!handler)
    {
        LOGPRINT_ERROR("Bad handler lookup (which_callset=%u).",
            (unsigned int)which_callset);
        reply_header->seqn = header->seqn;
        reply_header->status = StatusEnum_RPC_BAD_HANDLER_LOOKUP;
        *reply_encoded_size = Pb_pack(reply_buf,
                                      reply_buf_max_size,
                                      &rpc_reply_frame,
                                      info->frame_fields);
        return;
    }
    LOGPRINT_DEBUG("Got handler 0x%08x", (unsigned int)handler);

    /** @brief Call the handler. */
    uint8_t *call_frame = &rpc_call_frame[info->callset_offset];
    uint8_t *reply_frame = &rpc_reply_frame[info->callset_offset];
    handler(call_frame, reply_frame, &reply_header->status);

    if (header->no_reply)
    {
        return;
    }

    reply_header->seqn = header->seqn;
    rpc_reply_frame[0] = 1;        // set has_header in RpcFrame.
    rpc_reply_frame[info->which_callset_offset] = which_callset;
    *reply_encoded_size = Pb_pack(reply_buf,
                                  reply_buf_max_size,
                                  &rpc_reply_frame,
                                  info->frame_fields);
}
