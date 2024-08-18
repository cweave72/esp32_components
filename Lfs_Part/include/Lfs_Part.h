/*******************************************************************************
 *  @file: Lfs_Part.h
 *   
 *  @brief: Header for Lfs_Part, a littlefs wrapper using esp_partition api.
*******************************************************************************/
#ifndef LFS_PART_H
#define LFS_PART_H

#include "lfs.h"
#include "esp_partition.h"
#include "RtosUtils.h"
#include "CList.h"

/** @brief Minimum size of a block read.
    All read operations will be a multiple of this value. */
#ifndef LFS_PART_READ_SIZE
#define LFS_PART_READ_SIZE      128
#endif

/** @brief Minimum size of a block program.
    All write operations will be a multiple of this value. */
#ifndef LFS_PART_WRITE_SIZE
#define LFS_PART_WRITE_SIZE     128
#endif

/** @brief Size of a flash block in ESP32.*/
#define LFS_PART_BLOCK_SIZE     4096

/** @brief Size of block caches.
    Each cache buffers a portion of a block in RAM. The littlefs needs a read
    cache, a program cache, and one additional cache per file. Larger caches can
    improve performance by storing more data and reducing the number of disk
    accesses. Must be a multiple of the read and program sizes, and a factor of
    the block size (4096). */
#ifndef LFS_PART_CACHE_SIZE
#define LFS_PART_CACHE_SIZE     512
#endif

/** @brief Look ahead size. Must be a multiple of 8. */
#ifndef LFS_PART_LOOKAHEAD_SIZE
#define LFS_PART_LOOKAHEAD_SIZE     128
#endif

/** @brief Number of erase cycles before littlefs evicts metadata logs and moves 
    the metadata to another block. Suggested values are in the range 100-1000,
    with large values having better performance at the cost of less consistent
    wear distribution. Set to -1 to disable block-level wear-leveling. */
#ifndef LFS_PART_BLOCK_CYCLES
#define LFS_PART_BLOCK_CYCLES       512
#endif

/** @brief Context object for Lfs_Part.
*/
typedef struct Lfs_Part_t
{
    /** @brief Anchor for usage in a list (must be first). */
    CLIST_ANCHOR();

    /** @brief lfs filesystem. */
    lfs_t lfs;
    /** @brief Partition on which lfs is mounted. */
    const esp_partition_t *partition;
    /** @brief Rtos Mutex lock. */
    RTOS_MUTEX_STATIC_BUF lockbuf;
    RTOS_MUTEX lock;
    /** @brief lfs configuration object. */
    struct lfs_config cfg;
    /** @brief lfs file object. */
    lfs_file_t file;
    
} Lfs_Part_t;


/******************************************************************************
    [docexport Lfs_Part_register]
*//**
    @brief Registers a littlefs partition.
    Call this after Lfs_Part_init().
    @param[in] lpfs  Pointer to Lfs_Part_t object instance.
******************************************************************************/
void
Lfs_Part_register(Lfs_Part_t *lpfs);

/******************************************************************************
    [docexport Lfs_Part_getPartition]
*//**
    @brief Gets a partition object from the registry.
    @param[in] part_label  Label for partition to get.
    @return Returns the matching Lfs_Part_t object, NULL otherwise.
******************************************************************************/
Lfs_Part_t *
Lfs_Part_getPartition(const char *label);

/******************************************************************************
    [docexport Lfs_Part_init]
*//**
    @brief Initializes an Lfs_Part file system.
    @param[in] lpfs  Pointer to Lfs_Part_t object instance.
    @param[in] part_label  Label for partition to use.
    @param[out] lfs_result  lfs operation status.
******************************************************************************/
esp_err_t
Lfs_Part_init(Lfs_Part_t *lpfs, const char *part_label, int *lfs_result);
#endif
