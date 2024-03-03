/*******************************************************************************
 *  @file: UdpRpcServer.c
 *  
 *  @brief: UDP socket and RPC server.
*******************************************************************************/
#include "UdpRpcServer.h"
#include "UdpSocket.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

const char *TAG = "UdpRpcServer";

#define UDP_READ_TIMEOUT    10

/** @brief Raw UDP frame buffers. */
static uint8_t rcv_msg[PROTORPC_MSG_MAX_SIZE];
static uint8_t reply_msg[PROTORPC_MSG_MAX_SIZE];

/******************************************************************************
    rpc_callback
*//**
    @brief UdpServer callback. Handles RPC server interface.
    @param[in] server  Reference to the underlying TcpServer object.
    @param[in] sock  The accepted socket.
    @param[in] data  Pointer to data buffer to send.
    @param[in] len  Length of data to send.
******************************************************************************/
static void
rpc_callback(void *server, uint8_t *data, uint16_t len)
{
    /** @brief UdpRpcServer type masquerades as a UdpServer. */
    UdpRpcServer *udprpc_server = (UdpRpcServer *)server;
    ProtoRpc *rpc               = udprpc_server->rpc;
    UdpServer *udp_server       = &udprpc_server->udp_server;
    UdpSocket *udp              = &udp_server->udpsock;

    if (len > 0)
    {
        int num_sent;
        uint32_t reply_size;

        ProtoRpc_server(
            rpc,
            data,
            len,
            reply_msg,
            sizeof(reply_msg),
            &reply_size);

        if (reply_size > 0)
        {
            LOGPRINT_HEXDUMP_VERBOSE("Reply message.",
                reply_msg, reply_size);

            num_sent = UdpSocket_write(udp, reply_msg, reply_size);

            if (num_sent < 0)
            {
                LOGPRINT_ERROR("Error on rpc reply write.");
                return;
            }
            else if (num_sent != reply_size)
            {
                LOGPRINT_ERROR("Number of bytes send != reply_size.");
                return;
            }

            LOGPRINT_DEBUG("Wrote rpc reply: %d bytes.",
                (unsigned int)reply_size);
        }
    }
}

/******************************************************************************
    [docimport UdpRpcServer_init]
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
    uint8_t prio)
{
    server->rpc = rpc;

    /** @brief Initialize the UdpServer. */
    return UdpServer_init(
        &server->udp_server,
        port,
        rcv_msg,
        sizeof(rcv_msg),
        stack_size,
        "UDP Rpc",
        prio,
        UDP_READ_TIMEOUT,
        rpc_callback);
}
