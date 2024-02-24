/*******************************************************************************
 *  @file: TcpSocketServer.c
 *  
 *  @brief: Library implementing a tcp server.
*******************************************************************************/
#include <string.h>
#include "CheckCond.h"
#include "TcpServer.h"
#include "LogPrint.h"

/** @brief Allow local log print levels. */
#include "LogPrint_local.h"

static const char *TAG = "TcpServer";

#define KEEPALIVE_IDLE  5
#define KEEPALIVE_COUNT 3
#define KEEPALIVE_INTERVAL 5

/******************************************************************************
    send_socket
*//**
    @brief Sends data to socket.
    @param[in] sock  The accepted socket.
    @param[in] data  Pointer to data buffer to send.
    @param[in] len  Length of data to send.
    @returns Returns the number written or < 0 on error.
******************************************************************************/
static int
send_socket(int sock, uint8_t *data, uint16_t len)
{
    int num_written = 0;
    uint16_t to_write = len;

    while (num_written < len)
    {
        int num = send(sock, data + num_written, to_write, 0);
        if (num < 0)
        {
            LOGPRINT_ERROR("Error writing to socket: errno %d", errno);
            return num;
        }
        to_write -= num;
        num_written += num;
    }
    return num_written;
}

/******************************************************************************
    recv_socket
*//**
    @brief Receives data from socket. On recv data, calls user callback.
    @param[in] server  Pointer to TcpServer object.
    @param[in] sock  The accepted socket.
******************************************************************************/
static void
recv_socket(TcpServer *server, int sock)
{
    int len;

    do {
        /** @brief Read bytes at the port. */
        len = recv(sock, server->data, server->data_len, 0);

        if (len < 0)
        {
            LOGPRINT_ERROR("Error occured during socket recv: errno %d", errno);
        }
        else if (len == 0)
        {
            LOGPRINT_DEBUG("Peer connection closed.");
        }
        else
        {
            LOGPRINT_DEBUG("Received %u bytes.", len);
            LOGPRINT_HEXDUMP_VERBOSE("", server->data, len);

            /** @brief Call receive data callback. */
            if (server->cb)
            {
                server->cb((void *)server, sock, server->data, (uint16_t)len);
            }
        }
    } while (len > 0);
    
}

/******************************************************************************
    tcp_server_task
*//**
    @brief Main task loop for Tcp server.
******************************************************************************/
static void
tcp_server_task(void *p)
{
    TcpServer *server = (TcpServer *)p;
    TcpTask *task = &server->task;
    int sock, err;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    char addr_str[128];
    struct sockaddr_storage source_addr;
    socklen_t addr_len = sizeof(source_addr);

    LOGPRINT_INFO("Starting TcpServer Task: %s.", task->name);

    /** @brief Listen on port. */
    err = listen(server->sock, 1);
    if (err != 0)
    {
        LOGPRINT_ERROR("Error on socket listen: errno %d", errno);
        goto cleanup;
    }

    while (1)
    {
        LOGPRINT_DEBUG("Socket listening on port %u: %s",
            (unsigned int)server->port, task->name);

        /** @brief Accept incoming connections. */
        sock = accept(server->sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0)
        {
            LOGPRINT_ERROR("Error on socket accept: errno %d", errno);
            break;
        }

        /** @brief Set incoming socket keep-alive options. */
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

        TCPSOCKET_GET_ADDR(source_addr, addr_str);
        LOGPRINT_DEBUG("TCP connection accepted from %s", addr_str);

        /** @brief Receive socket data until error or disconnect. */
        recv_socket(server, sock);

        shutdown(sock, 0);
        close(sock);
    }

cleanup:
    close(server->sock);
    RTOS_TASK_DELETE(NULL);

}

/******************************************************************************
    [docimport TcpServer_init]
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
    TcpServer_cb *cb)
{
    struct sockaddr_in *dest_ip4 = &server->dest_addr;
    TcpTask *task = &server->task;
    int sock, err;
    esp_err_t ret;

    server->port = port;
    server->cb = cb;
    server->send = send_socket;

    task->stackSize = task_stackSize;
    task->prio = task_prio;
    strncpy(task->name, task_name, sizeof(task->name));

    dest_ip4->sin_addr.s_addr = (uint32_t)INADDR_ANY;  /* 0.0.0.0 */
    dest_ip4->sin_family = AF_INET;
    dest_ip4->sin_port = htons(port);   /* Note an endian swap occurs here */

    if (buf)
    {
        server->data = buf;
    }
    else
    {
        server->data = (uint8_t *)malloc(buf_len);
        CHECK_COND_RETURN_MSG(!server->data, -1, "Error allocating memory.");
    }
    server->data_len = buf_len;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    CHECK_COND_RETURN_MSG(sock < 0, sock, "Error creating socket.");
    LOGPRINT_INFO("Socket created successfully.");
    server->sock = sock;
    
    err = bind(sock, (struct sockaddr *)dest_ip4, sizeof(struct sockaddr_in));
    CHECK_COND_RETURN_MSG(err < 0, err, "Error binding socket.");
    LOGPRINT_INFO("Socket bound to port %d", server->port);

    ret = RTOS_TASK_CREATE(
        tcp_server_task,
        task->name,
        task->stackSize,
        (void *)server,
        task->prio,
        &task->handle);
    if (ret != ESP_OK)
    {
        LOGPRINT_ERROR("Failed creating tcp server task (%d)", ret);
        return ret;
    }

    return 0;
}
