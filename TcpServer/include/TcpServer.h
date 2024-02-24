/*******************************************************************************
 *  @file: TcpSocketServer.h
 *   
 *  @brief: Header for TcpSocketServer library.
*******************************************************************************/
#ifndef TCPSOCKETSERVER_H
#define TCPSOCKETSERVER_H

#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "LogPrint.h"
#include "RtosUtils.h"

/** @brief User callback. Receives data, can send data. */
typedef void TcpServer_cb(void *server, int sock, uint8_t *data, uint16_t len);

/** @brief Tcp Send function. */
typedef int TcpServer_send(int sock, uint8_t *data, uint16_t len);

/** @brief Parameters for the Tcp server task.
*/
typedef struct TcpTask
{
    /** @brief Task stack size. */
    uint16_t stackSize;
    /** @brief Task name. */
    char name[16];
    /** @brief Task prio. */
    uint8_t prio;
    /** @brief Loop task handle. */
    TaskHandle_t handle;
} TcpTask;

typedef struct TcpServer
{
    /** @brief Socket port number. */
    uint16_t port;
    /** @brief Pointer to data buffer. */
    uint8_t *data;
    /** @brief Length of data buffer. */
    uint16_t data_len;
    /** @brief User callback. */
    TcpServer_cb *cb;

    /** @brief Internal reference. */
    struct sockaddr_in dest_addr;
    struct sockaddr_storage source_addr;
    int sock;

    /** @brief Server send function. */
    TcpServer_send *send;
    /** @brief Tcp task object. */
    TcpTask task;
} TcpServer;

/** @brief Convert sender's IP to string. */
#define TCPSOCKET_GET_ADDR(sa, addr_str) \
    inet_ntoa_r(((struct sockaddr_in *)(&(sa)))->sin_addr, (addr_str), sizeof((addr_str))-1)


/******************************************************************************
    [docexport TcpServer_init]
*//**
    @brief Initializes a TCP server.
    @param[in] server  Pointer to uninitialized TcpServer object.
    @param[in] task  Pointer to *initialized* TcpTask object.
    @param[in] port  Port number to use.
    @param[in] buf  Pointer to user-allocated buffer used for Rx. If NULL,
    buffer will be dynamically allocated.
    @param[in] buf_len  Length of the buffer.
    @param[in] task_stackSize  Size of the server task stack.
    @param[in] task_name  Name for the task.
    @param[in] task_prio  Task priority.
    @param[in] cb  User callback.
    @return Returns 0 on success, negative on error.
******************************************************************************/
int
TcpServer_init(
    TcpServer *server,
    uint16_t port,
    uint8_t *buf,
    uint32_t buf_len,
    uint16_t task_stackSize,
    char *task_name,
    uint8_t task_prio,
    TcpServer_cb *cb);
#endif
