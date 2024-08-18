/*******************************************************************************
 *  @file: Lfs_Api.c
 *  
 *  @brief: API for interacting with an LFS file system.
*******************************************************************************/
#include "Lfs_Part.h"
#include "Lfs_Api.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "Lfs_Api";

#define CHECK_RETURN(call, ret)                               \
if ((ret) < 0)                                                \
{                                                             \
    LOGPRINT_ERROR("%s returned %d.", (call), (ret));         \
}

/******************************************************************************
    open
*//**
    @brief File open.
    @param[in] ctx  Context object holding Lfs_Part_t object.
    @param[in] path  File path.
    @param[in] flags  fs open flags (Fs_Api_open_flags;
        FSAPI_O_RDONLY,      Open a file as read only
        FSAPI_O_WRONLY,      Open a file as write only
        FSAPI_O_RDWR,        Open a file as read and write
        FSAPI_O_CREAT,       Create a file if it does not exist
        FSAPI_O_EXCL,        Fail if a file already exists
        FSAPI_O_TRUNC,       Truncate the existing file to zero size
        FSAPI_O_APPEND,      Move to end of file on every write
******************************************************************************/
static int
open(void *ctx, const char *path, int flags)
{
    Lfs_Part_t *lp = (Lfs_Part_t *)ctx;
    lfs_t *lfs = &lp->lfs;
    lfs_file_t *file = &lp->file;
    int _flags = 0;

    if (flags & FSAPI_O_RDONLY) _flags |= LFS_O_RDONLY;
    if (flags & FSAPI_O_WRONLY) _flags |= LFS_O_WRONLY;
    if (flags & FSAPI_O_RDWR)   _flags |= LFS_O_RDWR;
    if (flags & FSAPI_O_CREAT)  _flags |= LFS_O_CREAT;
    if (flags & FSAPI_O_EXCL)   _flags |= LFS_O_EXCL;
    if (flags & FSAPI_O_TRUNC)  _flags |= LFS_O_TRUNC;
    if (flags & FSAPI_O_APPEND) _flags |= LFS_O_APPEND;

    int ret = lfs_file_open(lfs, file, path, _flags);
    CHECK_RETURN("open", ret);
    return ret;
}

/******************************************************************************
    close
*//**
    @brief File close.
    @param[in] ctx  Context object holding Lfs_Part_t object.
    @param[in] fd  Unused.
******************************************************************************/
static int
close(void *ctx, int fd)
{
    Lfs_Part_t *lp = (Lfs_Part_t *)ctx;
    lfs_t *lfs = &lp->lfs;
    lfs_file_t *file = &lp->file;
    (void)fd;

    int ret = lfs_file_close(lfs, file);
    CHECK_RETURN("close", ret);
    return ret;
}

/******************************************************************************
    seek
*//**
    @brief File seek.
    @param[in] ctx  Context object holding Lfs_Part_t object.
    @param[in] fd  Unused.
    @param[in] off  Pointer to buffer to copy bytes into.
    @param[in] mode  Seek mode (Fs_Api_seek_flags)
******************************************************************************/
static int
seek(void *ctx, int fd, int off, int mode)
{
    Lfs_Part_t *lp = (Lfs_Part_t *)ctx;
    lfs_t *lfs = &lp->lfs;
    lfs_file_t *file = &lp->file;
    int whence;
    int new_off;
    (void)fd;

    switch (mode)
    {
    case FSAPI_SEEK_SET: whence = LFS_SEEK_SET; break;
    case FSAPI_SEEK_CUR: whence = LFS_SEEK_CUR; break;
    case FSAPI_SEEK_END: whence = LFS_SEEK_END; break;
    default: return 1;
    }
    new_off = lfs_file_seek(lfs, file, off, whence);
    CHECK_RETURN("seek", new_off);
    return new_off;
}

/******************************************************************************
    read
*//**
    @brief File read.
    @param[in] ctx  Context object holding Lfs_Part_t object.
    @param[in] fd  Unused.
    @param[in] buf  Pointer to buffer to copy bytes into.
    @param[in] size  Number of bytes to read.
******************************************************************************/
static int
read(void *ctx, int fd, char *buf, int size)
{
    Lfs_Part_t *lp = (Lfs_Part_t *)ctx;
    lfs_t *lfs = &lp->lfs;
    lfs_file_t *file = &lp->file;
    (void)fd;

    int ret = lfs_file_read(lfs, file, buf, size);
    CHECK_RETURN("read", ret);
    return ret;
}

/******************************************************************************
    write
*//**
    @brief File write.
    @param[in] ctx  Context object holding Lfs_Part_t object.
    @param[in] fd  Unused.
    @param[in] buf  Pointer to buffer holding bytes to write.
    @param[in] size  Number of bytes to write.
******************************************************************************/
static int
write(void *ctx, int fd, char *buf, int size)
{
    Lfs_Part_t *lp = (Lfs_Part_t *)ctx;
    lfs_t *lfs = &lp->lfs;
    lfs_file_t *file = &lp->file;
    (void)fd;

    int ret = lfs_file_write(lfs, file, buf, size);
    CHECK_RETURN("write", ret);
    return ret;
}

/******************************************************************************
    fsize
*//**
    @brief File size.
    @param[in] ctx  Context object holding Lfs_Part_t object.
    @param[in] fd  Unused.
    @return Returns the file size or negative on error.
******************************************************************************/
static int
fsize(void *ctx, int fd)
{
    Lfs_Part_t *lp = (Lfs_Part_t *)ctx;
    lfs_t *lfs = &lp->lfs;
    lfs_file_t *file = &lp->file;
    (void)fd;

    int ret = lfs_file_size(lfs, file);
    CHECK_RETURN("fsize", ret);
    return ret;
}

/******************************************************************************
    [docimport Lfs_Api_init]
*//**
    @brief Initializes an Lfs_Api object.
    @param[in] api  Pointer to the Fs_Api to be initialized.
    @param[in] part_label  Partition label.
******************************************************************************/
int
Lfs_Api_init(Fs_Api *api, const char *part_label)
{
    Lfs_Part_t *part = Lfs_Part_getPartition(part_label);
    if (!part)
    {
        LOGPRINT_ERROR("Error getting partition %s for Lfs_Api.", part_label);
        return -1;
    }

    api->ctx   = (void *)part;
    api->open  = open;
    api->close = close;
    api->read  = read;
    api->seek  = seek;
    api->write = write;
    api->fsize = fsize;

    return 0;
}
