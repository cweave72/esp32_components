/*******************************************************************************
 *  @file: UdpEcho.h
 *   
 *  @brief: Header for UdpEcho.
*******************************************************************************/
#ifndef UdpECHO_H
#define UdpECHO_H

#include <stdint.h>
#include "UdpServer.h"

/** @brief UdpEcho object.
*/
typedef struct UdpEcho
{
    /** @brief Server instance. */
    UdpServer udp_svr;
    /** @brief Total byte count. */
    uint32_t byte_count;
    
} UdpEcho;

/******************************************************************************
    [docexport UdpEcho_init]
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
    uint16_t timeout);
#endif
