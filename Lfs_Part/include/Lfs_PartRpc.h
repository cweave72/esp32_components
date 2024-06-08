/*******************************************************************************
 *  @file: Lfs_PartRpc.h
 *
 *  @brief: Header for Lfs_PartRpc.
*******************************************************************************/
#ifndef LFS_PARTRPC_H
#define LFS_PARTRPC_H

#include <stdint.h>
#include "Lfs_Part.h"
#include "ProtoRpc.h"

/******************************************************************************
    [docexport Lfs_PartRpc_init]
*//**
    @brief Initialization for the Lfs_PartRpc.
    @return Returns 0 on success, -1 on failure.
******************************************************************************/
int
Lfs_PartRpc_init(Lfs_Part_t *lpfs);

/******************************************************************************
    [docexport Lfs_PartRpc_resolver]
*//**
    @brief Resolver function for Lfs_PartRpc.
******************************************************************************/
ProtoRpc_handler *
Lfs_PartRpc_resolver(void *call_frame, uint32_t offset);
#endif
