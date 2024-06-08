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
    lfs_write_full_append
*//**
    @brief Full service appends to a file. Creates file if it does not exist.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] path  Path to file.
    @param[in] buf  Pointer to write buffer.
    @param[in] size  size to write.
    @return Returns 0 on success negative on error.
******************************************************************************/
#define lfs_write_full_append(lfs, path, buf, size)\
    lfs_write_full((lfs), (path), 0, LFS_SEEK_END, (buf), (size))

/******************************************************************************
    lfs_write_append
*//**
    @brief Appends to an open file.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] file  Open file descriptor.
    @param[in] buf  Pointer to write buffer.
    @param[in] size  size to write.
    @return Returns 0 on success negative on error.
******************************************************************************/
#define lfs_write_append(lfs, file, buf, size)\
    lfs_write_to_offset((lfs), (file), 0, LFS_SEEK_END, (buf), (size))

/******************************************************************************
    [docexport lfs_read_full]
*//**
    @brief Full service read. Opens a file, seeks to an offset, reads, closes.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] path  Path to file.
    @param[in] offset  Offset to read from.
    @param[in] whence  Whence seek flag. (0=SET; 1=CUR; 2=END)
    @param[in] buf  Pointer to read buffer.
    @param[in] size  size to read.
    @return Returns 0 on success negative on error.
******************************************************************************/
int
lfs_read_full(
    lfs_t *lfs,
    const char *path,
    uint32_t offset,
    int whence, 
    void *buf,
    uint32_t size);

/******************************************************************************
    [docexport lfs_read_from_offset]
*//**
    @brief Reads an open file from offset.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] file  Open file descriptor.
    @param[in] offset  Offset to read from.
    @param[in] whence  Whence seek flag. (0=SET; 1=CUR; 2=END)
    @param[in] buf  Pointer to read buffer.
    @param[in] size  size to read.
    @return Returns 0 on success negative on error.
******************************************************************************/
int
lfs_read_from_offset(
    lfs_t *lfs,
    lfs_file_t *file,
    uint32_t offset,
    int whence, 
    void *buf,
    uint32_t size);

/******************************************************************************
    [docexport lfs_write_full]
*//**
    @brief Full service write. Opens a file, seeks to an offset, writes, closes.
    Will create the file if it does not exist.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] path  Path to file.
    @param[in] offset  Offset to write to.
    @param[in] whence  Whence seek flag. (0=SET; 1=CUR; 2=END)
    @param[in] buf  Pointer to write buffer.
    @param[in] size  size to write.
    @return Returns num written on success negative on error.
******************************************************************************/
int
lfs_write_full(
    lfs_t *lfs,
    const char *path,
    uint32_t offset,
    int whence, 
    void *buf,
    uint32_t size);

/******************************************************************************
    [docexport lfs_write_to_offset]
*//**
    @brief Writes an open file at offset.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] file  Open file descriptor.
    @param[in] offset  Offset to write to.
    @param[in] whence  Whence seek flag. (0=SET; 1=CUR; 2=END)
    @param[in] buf  Pointer to write buffer.
    @param[in] size  size to write.
    @return Returns num written on success negative on error.
******************************************************************************/
int
lfs_write_to_offset(
    lfs_t *lfs,
    lfs_file_t *file,
    uint32_t offset,
    int whence, 
    const void *buf,
    uint32_t size);

/******************************************************************************
    [docexport lfs_exists]
*//**
    @brief Checks if the provided path exists.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] path  Path to check.
    @param[out] info  (optional) Returned info struct.
    @return Returns 0 if paths exists, negative if path doesn't exist.
******************************************************************************/
int
lfs_exists(lfs_t *lfs, const char *path, struct lfs_info *info);

/******************************************************************************
    [docexport lfs_pool_get_descriptor]
*//**
    @brief Gets the next available descriptor from the pool. Marks it as used.
    @param[in] lp  Pointer to the pool.
    @param[out] descr  Lfs descriptor.
    @return Returns file descriptor integer, -1 on none available.
******************************************************************************/
int
lfs_pool_get_descriptor(lfs_pool_t *lp, lfs_descriptor_t **descr);

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
