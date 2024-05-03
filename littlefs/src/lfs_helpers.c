/*******************************************************************************
 *  @file: lfs_helpers.c
 *  
 *  @brief: Helper functions for littlefs.
*******************************************************************************/
#include "lfs_helpers.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "lfs_helpers";

/******************************************************************************
    lookup_descriptor
*//**
    @brief Gets a descriptor from the pool at given index.
    @param[in] lp  Pointer to the pool.
    @param[in] idx  Index to get.
    @return Returns the pointer to the descriptor at pool index idx.
******************************************************************************/
static lfs_descriptor_t *
lookup_descriptor(lfs_pool_t *lp, uint8_t idx)
{
    if (idx < lp->num)
    {
        return lp->pool + idx;
    }
    return NULL;
}

/******************************************************************************
    [docimport lfs_exists]
*//**
    @brief Checks if the provided path exists.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] path  Path to check.
    @return Returns 0 if paths exists, negative if path doesn't exist.
******************************************************************************/
int
lfs_exists(lfs_t *lfs, const char *path)
{
    struct lfs_info info;
    return lfs_stat(lfs, path, &info);
}

/******************************************************************************
    [docimport lfs_pool_get_descriptor]
*//**
    @brief Gets the next available descriptor from the pool. Marks it as used.
    @param[in] lp  Pointer to the pool.
    @param[out] descr  Lfs descriptor.
    @return Returns file descriptor integer, -1 on none available.
******************************************************************************/
int
lfs_pool_get_descriptor(lfs_pool_t *lp, lfs_descriptor_t *descr)
{
    uint8_t idx;

    descr = NULL;
    for (idx = 0; idx < lp->num; idx++)
    {
        descr = &lp->pool[idx];
        if (descr->state == LFS_DESCRIPTOR_AVAIL)
        {
            LOGPRINT_DEBUG("Returing descriptor %u.", idx);
            descr->state = LFS_DESCRIPTOR_INUSE;
            return idx;
        }
    }
    LOGPRINT_ERROR("No available descriptors.");
    return -1;
}

/******************************************************************************
    [docimport lfs_pool_put_descriptor]
*//**
    @brief Releases a descriptor back to the pool.
    @param[in] lp  Pointer to the pool.
    @param[in] fd  Descriptor to release.
******************************************************************************/
void
lfs_pool_put_descriptor(lfs_pool_t *lp, int fd)
{
    lfs_descriptor_t *descr = lookup_descriptor(lp, fd);
    if (!descr)
    {
        LOGPRINT_ERROR("Invalid descriptor provided: %d", fd);
        return;
    }
    LOGPRINT_DEBUG("Releasing descriptor %u back to the pool.", fd);
    descr->state = LFS_DESCRIPTOR_AVAIL;
}

/******************************************************************************
    [docimport lfs_pool_init]
*//**
    @brief Initializes a pool of lfs_descriptors.
    @param[in] num Number of descriptors to allocate.
    @return Returns a pointer to the first descriptor.
******************************************************************************/
lfs_pool_t *
lfs_pool_init(uint8_t num)
{
    lfs_pool_t *lp;
    lfs_descriptor_t *pool;

    lp = malloc(sizeof(lfs_pool_t));
    if (!lp)
    {
        LOGPRINT_ERROR("Error allocating pool.");
        return NULL;
    }

    pool = malloc(num * sizeof(lfs_descriptor_t));
    if (!pool)
    {
        LOGPRINT_ERROR("Error allocating descriptors.");
        free(pool);
        return NULL;
    }

    /* Init the descriptors */
    for (lfs_descriptor_t *d = pool; d < pool + num; d++)
    {
        d->state = LFS_DESCRIPTOR_AVAIL;
    }

    lp->pool = pool;
    lp->num = num;
    return lp;
}
