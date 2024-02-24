/*******************************************************************************
 *  @file: TcpRpcServer.c
 *  
 *  @brief: Library for TCP-based Rpc server.
*******************************************************************************/
#include "TcpRpcServer.h"
#include "TcpServer.h"
#include "Cobs_frame.h"
#include "ProtoRpc.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "TcpRpcServer";

#define TCP_BUFFER_SIZE     1024

/** @brief Static buffers used for data. */
/* Buffer used to hold received socket data */
static uint8_t tcp_rx_buf[TCP_BUFFER_SIZE];
/* Buffer used to hold transmit socket data */
static uint8_t tcp_tx_buf[TCP_BUFFER_SIZE];
/* Buffer to hold the deframed (protobuf-packed) rpc received message. */
static uint8_t rpc_rcv_msg[PROTORPC_MSG_MAX_SIZE];
/* Buffer to hold the protobuf-packed rpc reply message. */
static uint8_t rpc_reply_msg[PROTORPC_MSG_MAX_SIZE];

/******************************************************************************
    rpc_callback
*//**
    @brief TcpServer callback. Handles RPC server interface.
    @param[in] server  Reference to the underlying TcpServer object.
    @param[in] sock  The accepted socket.
    @param[in] data  Pointer to data buffer to send.
    @param[in] len  Length of data to send.
******************************************************************************/
static void
rpc_callback(void *server, int sock, uint8_t *data, uint16_t len)
{
    /** @brief TcpRpcServer type masquerades as a TcpServer. */
    TcpRpcServer *tcprpc_server = (TcpRpcServer *)server;
    TcpServer_send *send        = tcprpc_server->tcp.send;
    ProtoRpc *rpc               = tcprpc_server->rpc;
    Cobs_Deframer *deframer     = &tcprpc_server->deframer;
    int raw_msg_size;
    int num_sent;
    uint32_t reply_size;

    /*  Attempt deframe of incoming stream. A positive raw_msg_size indicates a
        new decoded message is available.*/
    raw_msg_size = Cobs_deframer(
        deframer,
        data,
        len,
        rpc_rcv_msg,
        sizeof(rpc_rcv_msg));

    if (raw_msg_size)
    {
        LOGPRINT_HEXDUMP_VERBOSE("Deframed raw message.", rpc_rcv_msg, raw_msg_size);

        ProtoRpc_server(
            rpc,
            rpc_rcv_msg,
            raw_msg_size,
            rpc_reply_msg,
            sizeof(rpc_reply_msg),
            &reply_size);

        if (reply_size > 0)
        {
            int framed_size = Cobs_framer(
                rpc_reply_msg,
                reply_size,
                tcp_tx_buf,
                sizeof(tcp_tx_buf));

            LOGPRINT_HEXDUMP_VERBOSE("Framed Tx message.", tcp_tx_buf, framed_size);
            num_sent = send(sock, tcp_tx_buf, framed_size);
            LOGPRINT_DEBUG("Wrote rpc reply: %d bytes.", num_sent);
        }
    }
}


/******************************************************************************
    [docimport TcpRpcServer_init]
*//**
    @brief Initializes the TCP-based RPC server.
    @param[in] server  Pointer to uninitialized TcpRpcServer instance.
    @param[in] rpc  Pointer to *initialized* ProtoRpc instance.
    @param[in] port  Port number to use.
    @param[in] stack_size  Size of the server task stack.
    @param[in] prio  Server task priority.
    @return Returns 0 on success, negative on error.
******************************************************************************/
int
TcpRpcServer_init(
    TcpRpcServer *server,
    ProtoRpc *rpc,
    uint16_t port,
    uint16_t stack_size,
    uint8_t prio)
{
    server->rpc = rpc;

    /** @brief Initialize the Deframer. */
    Cobs_deframer_init(&server->deframer, sizeof(tcp_rx_buf));

    /** @brief Initialize the TcpServer. */
    return TcpServer_init(
        &server->tcp,
        port,
        tcp_rx_buf,
        sizeof(tcp_rx_buf),
        stack_size,
        "TCP Rpc",
        prio,
        rpc_callback);
}
