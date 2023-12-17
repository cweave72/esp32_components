/*******************************************************************************
 *  @file: UdpSocket.h
 *   
 *  @brief: Header for UdpSocket.
*******************************************************************************/
#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "lwip/sockets.h"

typedef struct UdpSocket
{
    uint16_t port;
    /** @brief Pointer to data buffer. */
    char *data;
    /** @brief Length of data buffer. */
    uint32_t data_len;
    /** @brief Internal reference. */
    struct sockaddr_in dest_addr;
    struct sockaddr_storage source_addr;
    int sock;
} UdpSocket;

/** @brief Convert sender's IP to string. */
#define UDPSOCKET_GET_ADDR(sa, addr_str) \
    inet_ntoa_r(((struct sockaddr_in *)(&(sa)))->sin_addr, (addr_str), sizeof((addr_str))-1)

/******************************************************************************
    [docexport UdpSocket_read]
*//**
    @brief Reads from an initialzed socket.  Call this from task thread.
    @param[in] udp_sock  Pointer to UdpSocket object.
    @param[in] buffer  Pointer to buffer.
    @param[in] size  Size of buffer.
    @return Returns positive length on success, negative error code on failure.
******************************************************************************/
int
UdpSocket_read(UdpSocket *udp_sock, char *buffer, uint32_t size);

/******************************************************************************
    [docexport UdpSocket_write]
*//**
    @brief Writes to a UDP socket.
    @param[in] udp_sock  Pointer to UdpSocket object.
    @param[in] buffer  Pointer to buffer to send.
    @param[in] size  Size of buffer to send.
    @return Returns 0 on success, negative on failure.
******************************************************************************/
int
UdpSocket_write(UdpSocket *udp_sock, char *buffer, uint32_t size);

/******************************************************************************
    [docexport UdpSocket_init]
*//**
    @brief Initializes a UDP socket.
    @param[in] udp_sock  Pointer to UdpSocket object.
    @param[in] port  Port number to bind to.
    @param[in] timeout  Socket timeout in s.
    @return Returns negative error code.
******************************************************************************/
int
UdpSocket_init(UdpSocket *udp_sock, uint16_t port, uint16_t timeout);
#endif
