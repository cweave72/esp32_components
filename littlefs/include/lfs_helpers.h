/*******************************************************************************
 *  @file: lfs_helpers.h
 *   
 *  @brief: Helper functions for littlefs.
*******************************************************************************/
#ifndef LFS_HELPERS_H
#define LFS_HELPERS_H

#include <stdint.h>
#include "lfs.h"

typedef enum lfs_descriptor_state_t
{
    LFS_DESCRIPTOR_AVAIL = 0,
    LFS_DESCRIPTOR_INUSE
} lfs_descriptor_state_t;

/** @brief An lfs open file/directory descriptor.
*/
typedef struct lfs_descriptor_t
{
    /* lfs objects */
    lfs_file_t file;
    lfs_dir_t dir;
    /* State of the descriptor */
    lfs_descriptor_state_t state;
} lfs_descriptor_t;

/** @brief A pool of descriptors.
*/
typedef struct lfs_pool_t
{
    /* Pointer to the pool of descriptors. */
    lfs_descriptor_t *pool;
    /* Number of descriptors in the pool. */
    uint8_t num;
} lfs_pool_t;

/******************************************************************************
    [docexport lfs_exists]
*//**
    @brief Checks if the provided path exists.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] path  Path to check.
    @return Returns 0 if paths exists, negative if path doesn't exist.
******************************************************************************/
int
lfs_exists(lfs_t *lfs, const char *path);

/******************************************************************************
    [docexport lfs_pool_get_descriptor]
*//**
    @brief Gets the next available descriptor from the pool. Marks it as used.
    @param[in] lp  Pointer to the pool.
    @param[out] descr  Lfs descriptor.
    @return Returns file descriptor integer, -1 on none available.
******************************************************************************/
int
lfs_pool_get_descriptor(lfs_pool_t *lp, lfs_descriptor_t *descr);

/******************************************************************************
    [docexport lfs_pool_put_descriptor]
*//**
    @brief Releases a descriptor back to the pool.
    @param[in] lp  Pointer to the pool.
    @param[in] fd  Descriptor to release.
******************************************************************************/
void
lfs_pool_put_descriptor(lfs_pool_t *lp, int fd);

/******************************************************************************
    [docexport lfs_pool_init]
*//**
    @brief Initializes a pool of lfs_descriptors.
    @param[in] num Number of descriptors to allocate.
    @return Returns a pointer to the first descriptor.
******************************************************************************/
lfs_pool_t *
lfs_pool_init(uint8_t num);
#endif
