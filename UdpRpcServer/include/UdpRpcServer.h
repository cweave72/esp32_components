/*******************************************************************************
 *  @file: UdpRpcServer.h
 *   
 *  @brief: Header for UdpRpcServer.
*******************************************************************************/
#ifndef UDPRPCSERVER_H
#define UDPRPCSERVER_H

#include <stdint.h>
#include "UdpServer.h"
#include "ProtoRpc.h"

/** @brief TcpRpcServer object.
*/
typedef struct UdpRpcServer
{
    /** @brief Server instance. */
    UdpServer udp_server;
    /** @brief Pointer to the ProtoRpc instance. */
    ProtoRpc *rpc;
    
} UdpRpcServer;


/******************************************************************************
    [docexport UdpRpcServer_init]
*//**
    @brief Initializes the UDP-based RPC server.
    @param[in] server  Pointer to uninitialized UdpRpcServer instance.
    @param[in] rpc  Pointer to *initialized* ProtoRpc instance.
    @param[in] port  Port number to use.
    @param[in] stack_size  Size of the server task stack.
    @param[in] prio  Server task priority.
    @return Returns 0 on success, negative on error.
******************************************************************************/
int
UdpRpcServer_init(
    UdpRpcServer *server,
    ProtoRpc *rpc,
    uint16_t port,
    uint16_t stack_size,
    uint8_t prio);
#endif
