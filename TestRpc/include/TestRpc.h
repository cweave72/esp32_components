/*******************************************************************************
 *  @file: TestRpc.h
 *   
 *  @brief: Header for TestRpc.
*******************************************************************************/
#ifndef TESTRPC_H
#define TESTRPC_H

#include <stdint.h>

/******************************************************************************
    [docexport TestRpc_resolver]
*//**
    @brief Resolver function for TestRpc.
******************************************************************************/
ProtoRpc_handler *
TestRpc_resolver(void *call_frame, uint32_t offset);
#endif
