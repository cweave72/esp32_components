/*******************************************************************************
 *  @file: Lfs_PartRpc.h
 *
 *  @brief: Header for Lfs_PartRpc.
*******************************************************************************/
#ifndef LFS_PARTRPC_H
#define LFS_PARTRPC_H

#include <stdint.h>

/******************************************************************************
    [docexport Lfs_PartRpc_resolver]
*//**
    @brief Resolver function for Lfs_PartRpc.
******************************************************************************/
ProtoRpc_handler *
Lfs_PartRpc_resolver(void *call_frame, uint32_t offset);
#endif