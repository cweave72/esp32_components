/*******************************************************************************
 *  @file: Lfs_Api.h
 *   
 *  @brief: API for accessing an LFS filesystem.
*******************************************************************************/
#ifndef LFS_API_H
#define LFS_API_H
#include "Fs_Api.h"

/******************************************************************************
    [docexport Lfs_Api_init]
*//**
    @brief Initializes an Lfs_Api object.
    @param[in] api  Pointer to the Fs_Api to be initialized.
    @param[in] part_label  Partition label.
******************************************************************************/
int
Lfs_Api_init(Fs_Api *api, const char *part_label);
#endif
