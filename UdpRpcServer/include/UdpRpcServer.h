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
    @param[in] rpc  Pointer to RPC info object.
    @param[in] task_stack_size  Stack size for the task to be created.
******************************************************************************/
int
UdpRpcServer_Task_init(ProtoRpc *rpc, uint32_t task_stack_size);
#endif
