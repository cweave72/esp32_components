/*******************************************************************************
 *  @file: UdpServer.c
 *  
 *  @brief: Library implementing a udp server.
*******************************************************************************/
#include <string.h>
#include "CheckCond.h"
#include "UdpServer.h"
#include "LogPrint.h"

/** @brief Allow local log print levels. */
#include "LogPrint_local.h"

static const char *TAG = "UdpServer";

/******************************************************************************
    udp_server_task
*//**
    @brief Main task loop for Udp server.
******************************************************************************/
static void
udp_server_task(void *p)
{
    UdpServer *server = (UdpServer *)p;
    UdpSocket *udp = &server->udpsock;
    UdpTask *task = &server->task;

    LOGPRINT_DEBUG("Socket accepting connections on port %u: %s",
        (unsigned int)udp->port, task->name);

    while (1)
    {
        int num_read;

        /** @brief Receive socket data. */
        num_read = UdpSocket_read(udp, server->data, server->data_len);

        if (num_read < 0)
        {
            LOGPRINT_ERROR("Closing socket due to read error.");
            RTOS_TASK_SLEEP_s(1);
            continue;
        }
        else if (num_read > 0)
        {
            /** @brief Call callback to allow rx and tx on socket. */
            server->cb(
                (void *)server,
                server->data,
                (uint16_t)num_read);
        }
        else
        {
            /* Timeout on read. */
        }
    } /* end outer while */
}


/******************************************************************************
    [docimport UdpServer_init]
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
    UdpServer_cb *cb)
{
    UdpSocket *udp = &server->udpsock;
    UdpTask *task = &server->task;
    esp_err_t ret;
    int rc;

    CHECK_COND_RETURN_MSG(!cb, -1, "A callback must be provided.");
    server->cb = cb;

    task->stackSize = task_stackSize;
    task->prio = task_prio;
    strncpy(task->name, task_name, sizeof(task->name));

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

    rc = UdpSocket_init(udp, port, timeout);
    CHECK_COND_RETURN_MSG(rc < 0, rc, "Error initializing server.");

    ret = RTOS_TASK_CREATE(
        udp_server_task,
        task->name,
        task->stackSize,
        (void *)server,
        task->prio,
        &task->handle);
    if (ret != ESP_OK)
    {
        LOGPRINT_ERROR("Failed creating udp server task (%d)", ret);
        return ret;
    }

    return 0;
}
