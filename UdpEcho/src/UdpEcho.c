/*******************************************************************************
 *  @file: UdpEcho.c
 *  
 *  @brief: Simple Udp echo server.
*******************************************************************************/
#include "UdpSocket.h"
#include "UdpEcho.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "UdpEcho";

/******************************************************************************
    echo
*//**
    @brief UdpServer callback. Echos data received back to the socket.

    @param[in] server  Reference to the underlying UdpServer object.
    @param[in] data  Pointer to data buffer to send.
    @param[in] len  Length of data to send.
******************************************************************************/
static void
echo_callback(void *server, uint8_t *data, uint16_t len)
{
    /** @brief UdpEcho type masquerades as a UdpServer. */
    UdpEcho *echo_server = (UdpEcho *)server;
    UdpServer *udp_server = &echo_server->udp_svr;
    UdpSocket *udp = &udp_server->udpsock;
    int num;

    num = UdpSocket_write(udp, data, len);
    if (num < 0)
    {
        LOGPRINT_ERROR("Error on socket write: %d", num);
        return;
    }

    echo_server->byte_count += num;

    LOGPRINT_INFO("UDP Echo'd %d bytes (total: %u).", num,
        (unsigned int)echo_server->byte_count);
}

/******************************************************************************
    [docimport UdpEcho_init]
*//**
    @brief Initializes the echo server.

    @param[in] echo  Pointer to UdpEcho instance.
    @param[in] port  Port number to use.
    @param[in] buf  Pointer to user-allocated buffer used for Rx. If NULL,
    buffer will be dynamically allocated.
    @param[in] buf_len  Length of the buffer.
    @param[in] stack_size  Size of the server task stack.
    @param[in] name  Name for the task.
    @param[in] prio  Task priority.
    @param[in] timeout  Socket timeout, sec.
    @return Returns 0 on success, negative on error.
******************************************************************************/
int
UdpEcho_init(
    UdpEcho *echo,
    uint16_t port,
    uint8_t *buf,
    uint32_t buf_len,
    uint16_t stack_size,
    char *name,
    uint8_t prio,
    uint16_t timeout)
{
    echo->byte_count = 0;

    /** @brief Initialize the UdpServer. */
    return UdpServer_init(
        &echo->udp_svr,
        port,
        buf,
        buf_len,
        stack_size,
        name,
        prio,
        timeout,
        echo_callback);
}
