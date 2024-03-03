#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "lwip/err.h"
#include "UdpSocket.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "UdpSocket";

/** @brief Convert sender's IP to string. */
#define UDPSOCKET_GET_ADDR(sa, addr_str)                    \
    inet_ntoa_r(((struct sockaddr_in *)(&(sa)))->sin_addr,  \
        (addr_str), sizeof((addr_str))-1)

/******************************************************************************
    [docimport UdpSocket_read]
*//**
    @brief Reads from an initialzed socket.  Call this from a task thread.
    Source address is stored in udp_sock->source_addr member on return.

    @param[in] udp_sock  Pointer to UdpSocket object.
    @param[in] buffer  Pointer to buffer.
    @param[in] size  Size of buffer.
    @return Returns positive length on success, negative error code on failure.
******************************************************************************/
int
UdpSocket_read(UdpSocket *udp_sock, uint8_t *buffer, uint32_t size)
{
    struct sockaddr_storage *source_addr = &udp_sock->source_addr;
    socklen_t socklen = sizeof(struct sockaddr_storage);
    int len;

    len = recvfrom(udp_sock->sock, buffer, size, 0,
        (struct sockaddr *)source_addr, &socklen);
    if (len < 0)
    {
        if (errno != EAGAIN)
        {
            LOGPRINT_ERROR("recvfrom failed: errno %d", errno);
            return len;
        }
        else
        {
            /* Read timeout */
            return 0;
        }
    }

#ifdef LOCAL_DEBUG
    char addr_str[128];
    UDPSOCKET_GET_ADDR(*source_addr, addr_str);
    LOGPRINT_DEBUG("UDP read %d bytes from %s", len, addr_str);
#endif
    return len;
}

/******************************************************************************
    [docimport UdpSocket_write]
*//**
    @brief Writes to a UDP socket.
    Writes to the address specified in udp_sock->source_addr.

    @param[in] udp_sock  Pointer to UdpSocket object.
    @param[in] buffer  Pointer to buffer to send.
    @param[in] size  Size of buffer to send.
    @return Returns the number of bytes written on success, negative on failure.
******************************************************************************/
int
UdpSocket_write(UdpSocket *udp_sock, uint8_t *buffer, uint32_t size)
{
    struct sockaddr_storage *source_addr = &udp_sock->source_addr;
    socklen_t socklen = sizeof(struct sockaddr_storage);
    int ret;

    ret = sendto(udp_sock->sock, buffer, size, 0,
        (struct sockaddr *)source_addr, socklen);
    if (ret < 0)
    {
        LOGPRINT_ERROR("Error occurred during writing: errno %d", errno);
        return ret;
    }

    return ret;
}

/******************************************************************************
    [docimport UdpSocket_init]
*//**
    @brief Initializes a UDP socket.
    @param[in] udp_sock  Pointer to UdpSocket object.
    @param[in] port  Port number to bind to.
    @param[in] timeout  Socket timeout in s.
    @return Returns negative error code.
******************************************************************************/
int
UdpSocket_init(UdpSocket *udp_sock, uint16_t port, uint16_t timeout)
{
    struct sockaddr_in *dest_ip4 = &udp_sock->dest_addr;
    struct timeval timv; 
    int sock, err;

    udp_sock->port = port;

    dest_ip4->sin_addr.s_addr = (uint32_t)INADDR_ANY;  /* 0.0.0.0 */
    dest_ip4->sin_family = AF_INET;
    dest_ip4->sin_port = htons(port);   /* Note an endian swap occurs here */

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return sock;
    }
    LOGPRINT_INFO("Socket created successfully.");
    udp_sock->sock = sock;

    /* Set timeout */
    timv.tv_sec = timeout;
    timv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timv, sizeof(timv));

    err = bind(sock, (struct sockaddr *)dest_ip4, sizeof(struct sockaddr_in));
    if (err < 0)
    {
        LOGPRINT_ERROR("Socket unable to bind: errno %d", errno);
        return err;
    }
    LOGPRINT_INFO("Socket bound, port %d", udp_sock->port);

    return 0;
}

