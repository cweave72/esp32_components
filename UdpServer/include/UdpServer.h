/*******************************************************************************
 *  @file: UdpServer.h
 *   
 *  @brief: Header for UdpServer.
*******************************************************************************/
#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "UdpSocket.h"
#include "LogPrint.h"
#include "RtosUtils.h"

/******************************************************************************
    UdpServer_cb
*//**
    @brief Server user callback.

    @param[in] server  Pointer to the server object.
    @param[in] data  Pointer to buffer holding received data.
    @param[in] len  Length of received data.
    @param[out] finished  Status:
    0 --> not finished, keep connection active
    1 --> finished, close connection.
******************************************************************************/
typedef void
UdpServer_cb(
    void *server,
    uint8_t *data,
    uint16_t len);

/** @brief Parameters for the Udp server task.
*/
typedef struct UdpTask
{
    /** @brief Task stack size. */
    uint16_t stackSize;
    /** @brief Task name. */
    char name[16];
    /** @brief Task prio. */
    uint8_t prio;
    /** @brief Loop task handle. */
    TaskHandle_t handle;

} UdpTask;

typedef struct UdpServer
{
    /** @brief Udp socket instance. */
    UdpSocket udpsock;

    /** @brief Socket port number. */
    uint16_t port;
    /** @brief Pointer to data buffer. */
    uint8_t *data;
    /** @brief Length of data buffer. */
    uint16_t data_len;
    /** @brief User callback. */
    UdpServer_cb *cb;

    /** @brief Internal reference. */
    struct sockaddr_in dest_addr;
    struct sockaddr_storage source_addr;
    int sock;

    /** @brief Server send function. */
    //UdpServer_send *send;
    /** @brief Udp task object. */
    UdpTask task;

} UdpServer;

/** @brief Convert sender's IP to string. */
#define UDPSOCKET_GET_ADDR(sa, addr_str) \
    inet_ntoa_r(((struct sockaddr_in *)(&(sa)))->sin_addr, (addr_str), sizeof((addr_str))-1)


/******************************************************************************
    [docexport UdpServer_init]
*//**
    @brief Initializes a UDP server.
    @param[in] server  Pointer to uninitialized UdpServer object.
    @param[in] task  Pointer to *initialized* UdpTask object.
    @param[in] port  Port number to use.
    @param[in] buf  Pointer to user-allocated buffer used for Rx. If NULL,
    buffer will be dynamically allocated.
    @param[in] buf_len  Length of the buffer.
    @param[in] task_stackSize  Size of the server task stack.
    @param[in] task_name  Name for the task.
    @param[in] task_prio  Task priority.
    @param[in] timeout  Socket timeout, sec.
    @param[in] cb  User callback.
    @return Returns 0 on success, negative on error.
******************************************************************************/
int
UdpServer_init(
    UdpServer *server,
    uint16_t port,
    uint8_t *buf,
    uint32_t buf_len,
    uint16_t task_stackSize,
    char *task_name,
    uint8_t task_prio,
    uint16_t timeout,
    UdpServer_cb *cb);
#endif
