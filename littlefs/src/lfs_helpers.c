/*******************************************************************************
 *  @file: lfs_helpers.c
 *  
 *  @brief: Helper functions for littlefs.
*******************************************************************************/
#include "lfs_helpers.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "lfs_helpers";

#define RETURN_ON_FAIL(ret) \
do {                        \
    if ((ret) < 0) {        \
        return ret;         \
    }                       \
} while(0)

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
    read_from_offset
*//**
    @brief Reads a file from a given offset.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] file  Pointer to the open file descriptor.
    @param[in] offset  Offset to read from.
    @param[in] whence  Whence seek flag. (0=SET; 1=CUR; 2=END)
    @param[in] buf  Pointer to read buffer.
    @param[in] size  size to read.
    @return Returns 0 on success negative on error.
******************************************************************************/
static int
read_from_offset(
    lfs_t *lfs,
    lfs_file_t *file,
    uint32_t offset,
    int whence, 
    void *buf,
    uint32_t size)
{
    int ret;
    ret = lfs_file_seek(lfs, file, offset, whence);
    RETURN_ON_FAIL(ret);
    ret = lfs_file_read(lfs, file, buf, size);
    return ret;
}

/******************************************************************************
    write_to_offset
*//**
    @brief Writes a file to a given offset.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] file  Pointer to the open file descriptor.
    @param[in] offset  Offset to read from.
    @param[in] whence  Whence seek flag. (0=SET; 1=CUR; 2=END)
    @param[in] buf  Pointer to write buffer.
    @param[in] size  size to write.
    @return Returns num written on success negative on error.
******************************************************************************/
static int
write_to_offset(
    lfs_t *lfs,
    lfs_file_t *file,
    uint32_t offset,
    int whence, 
    const void *buf,
    uint32_t size)
{
    int ret;
    ret = lfs_file_seek(lfs, file, offset, whence);
    RETURN_ON_FAIL(ret);
    ret = lfs_file_write(lfs, file, buf, size);
    return ret;
}

/******************************************************************************
    [docimport lfs_read_full]
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
    uint32_t size)
{
    lfs_file_t file;
    int ret;
    
    ret = lfs_file_open(lfs, &file, path, LFS_O_RDONLY);
    RETURN_ON_FAIL(ret);
    ret = read_from_offset(lfs, &file, offset, whence, buf, size);
    RETURN_ON_FAIL(ret);
    ret = lfs_file_close(lfs, &file);
    return ret;
}

/******************************************************************************
    [docimport lfs_read_from_offset]
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
    uint32_t size)
{
    int ret = read_from_offset(lfs, file, offset, whence, buf, size);
    return ret;
}

/******************************************************************************
    [docimport lfs_write_full]
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
    uint32_t size)
{
    lfs_file_t file;
    int ret;
    
    ret = lfs_file_open(lfs, &file, path, LFS_O_CREAT|LFS_O_WRONLY);
    RETURN_ON_FAIL(ret);
    ret = write_to_offset(lfs, &file, offset, whence, buf, size);
    RETURN_ON_FAIL(ret);
    ret = lfs_file_close(lfs, &file);
    return ret;
}

/******************************************************************************
    [docimport lfs_write_to_offset]
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
    uint32_t size)
{
    int ret = write_to_offset(lfs, file, offset, whence, buf, size);
    return ret;
}

/******************************************************************************
    [docimport lfs_exists]
*//**
    @brief Checks if the provided path exists.
    @param[in] lfs  Pointer to the file system instance.
    @param[in] path  Path to check.
    @param[out] info  (optional) Returned info struct.
    @return Returns 0 if paths exists, negative if path doesn't exist.
******************************************************************************/
int
lfs_exists(lfs_t *lfs, const char *path, struct lfs_info *info)
{
    struct lfs_info _info;
    struct lfs_info *pinfo = (info) ? info : &_info;
    return lfs_stat(lfs, path, pinfo);
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
lfs_pool_get_descriptor(lfs_pool_t *lp, lfs_descriptor_t **descr)
{
    uint8_t idx;
    lfs_descriptor_t *d;

    *descr = NULL;
    for (idx = 0; idx < lp->num; idx++)
    {
        d = &lp->pool[idx];
        if (!d)
        {
            LOGPRINT_ERROR("NULL descriptor at pool[%u]!", (unsigned int)idx);
            return -1;
        }

        if (d->state == LFS_DESCRIPTOR_AVAIL)
        {
            LOGPRINT_DEBUG("Returing descriptor %u.", idx);
            d->state = LFS_DESCRIPTOR_INUSE;
            *descr = d;
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
