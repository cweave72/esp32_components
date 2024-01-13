/*******************************************************************************
 *  @file: UdpRpcServer.h
 *   
 *  @brief: Header for UdpRpcServer.
*******************************************************************************/
#ifndef UDPRPCSERVER_H
#define UDPRPCSERVER_H

#include <stdint.h>
#include "ProtoRpc.h"



/******************************************************************************
    [docexport UdpRpcServer_Task_init]
*//**
    @brief Task initializer for the UDP socket and RPC server.
******************************************************************************/
int
UdpRpcServer_Task_init(
    ProtoRpc_info *info,
    ProtoRpc_resolvers resolvers,
    uint32_t num_resolvers,
    uint32_t task_stack_size);
#endif
