/*******************************************************************************
 *  @file: UdpRpcServer.c
 *  
 *  @brief: UDP socket and RPC server.
*******************************************************************************/
#include "UdpSocket.h"
#include "RtosUtils.h"
#include "UdpRpcServer.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

#define UDP_READ_TIMEOUT    10
#define RPC_SOCKET_PORT     13000
#define RPC_TASK_PRIO       10

/** @brief Static objects used by the server. */
static ProtoRpc *rpc_info;

/** @brief Raw UDP frame buffers. */
static uint8_t rcv_msg[PROTORPC_MSG_MAX_SIZE];
static uint8_t reply_msg[PROTORPC_MSG_MAX_SIZE];

static UdpSocket udp_sock;
static TaskHandle_t rpcTaskHandle;

const char *TAG = "UdpRpcServer";

/******************************************************************************
    rpc_server
*//**
    @brief Task function for the RPC server.
******************************************************************************/
static void
rpc_server(void *p)
{
    int ret;
    char addr_str[128];

    (void)p;

    LOGPRINT_INFO("UDP Rpc server task running.");

    ret = UdpSocket_init(&udp_sock, RPC_SOCKET_PORT, UDP_READ_TIMEOUT);
    if (ret < 0)
    {
        LOGPRINT_ERROR("Error initializing socket: %d", ret);
        return;
    }

    while (1)
    {
        int len, ret;
        uint32_t reply_size;

        len = UdpSocket_read(&udp_sock, (char *)rcv_msg, sizeof(rcv_msg));
        if (len > 0)
        {
            UDPSOCKET_GET_ADDR(udp_sock.source_addr, addr_str);
            LOGPRINT_DEBUG("Received %d bytes from %s:", len, addr_str);

            ProtoRpc_server(
                rpc_info,
                rcv_msg,
                len,
                reply_msg,
                sizeof(reply_msg),
                &reply_size);

            if (reply_size > 0)
            {
                ret = UdpSocket_write(&udp_sock, (char *)reply_msg, reply_size);
                if (ret == 0)
                {
                    LOGPRINT_DEBUG("Wrote %d bytes.", (unsigned int)reply_size);
                }
            }
        }
        else if (len == 0)
        {
            continue;
        }
    }
}

/******************************************************************************
    [docimport UdpRpcServer_Task_init]
*//**
    @brief Task initializer for the UDP socket and RPC server.
    @param[in] rpc  Pointer to RPC info object.
    @param[in] task_stack_size  Stack size for the task to be created.
******************************************************************************/
int
UdpRpcServer_Task_init(ProtoRpc *rpc, uint32_t task_stack_size)
{
    rpc_info = rpc;

    return RTOS_TASK_CREATE(
        rpc_server,
        "UDP Rpc",
        task_stack_size,
        NULL,
        RPC_TASK_PRIO,
        &rpcTaskHandle);
}
