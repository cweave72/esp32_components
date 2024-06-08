/*******************************************************************************
 *  @file: lfspart_rpc.c
 *
 *  @brief: Handlers for lfspart_rpc.
*******************************************************************************/
#include "Lfs_PartRpc.h"
#include "CList.h"
#include "Lfs_PartRpc.pb.h"
#include "ProtoRpc.pb.h"
#include "lfs_helpers.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "Lfs_PartRpc";

/** @brief MAX number of simultaneously open file descriptors. */
#define MAX_OPEN_DESCRIPTORS    4

/** @brief Pool of descriptors. */
static lfs_pool_t *pool = NULL;

/** @brief Local copy of the LFS partition FS. */
static Lfs_Part_t *_lpfs = NULL;

typedef struct CacheItem
{
    /** @brief List anchor so that this object can be added to a list.
        MUST BE THE FIRST MEMBER */
    CLIST_ANCHOR();

    /** @brief The fd. */
    uint8_t fd;
    /** @brief lfs fs reference. */
    lfs_t *lfs;
    /** @brief The lfs descriptor. */
    lfs_descriptor_t *descr;
    /** @brief item info */
    struct lfs_info info;
} CacheItem;

/** @brief The cache holding open fd's. */
static CList cache;

#define MIN(x,y)  (((x) < (y)) ? (x) : (y))

/******************************************************************************
    cache_print
*//**
    @brief Prints all items in cache.
******************************************************************************/
static void
cache_print(void)
{
    CacheItem *item;
    LOGPRINT_DEBUG("Cache contents:");
    CLIST_ITER_ENTRY(item, &cache)
    {
        LOGPRINT_DEBUG("  %u: %s", (unsigned int)item->fd, item->info.name);
    }
}

/******************************************************************************
    cache_find_fd
*//**
    @brief Retrieves entry from cache matching fd.
    @param[in] fd  File descriptor to match.
    Returns the CacheItem matching fd, NULL if not found.
******************************************************************************/
static CacheItem *
cache_find_fd(uint8_t fd)
{
    CacheItem *item;
    CLIST_ITER_ENTRY(item, &cache)
    {
        if (item->fd == fd)
        {
            LOGPRINT_DEBUG("Found fd=%u in cache.", (unsigned int)fd);
            return item;
        }
    }
    LOGPRINT_ERROR("Did not find fd=%u in cache.", (unsigned int)fd);
    return NULL;
}

/******************************************************************************
    cache_rm_fd
*//**
    @brief Removed entry from cache matching fd. Releases back to the pool.
    @param[in] fd  File descriptor to match.
    Returns 0 on success, -1 on failure.
******************************************************************************/
static int
cache_rm_fd(uint8_t fd)
{
    CacheItem *item;
    bool found = false;

    CLIST_ITER_ENTRY(item, &cache)
    {
        if (item->fd == fd)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
#ifdef LOCAL_DEBUG
        cache_print();
#endif
        /* Release descriptor back to the pool */
        lfs_pool_put_descriptor(pool, fd);
        /* Remove from the cache of open fd's. */
        CList_remove((CList *)item);
        /* Finally, free the item */
        free(item);
        LOGPRINT_DEBUG("Removed fd=%u from cache.", (unsigned int)fd);
#ifdef LOCAL_DEBUG
        cache_print();
#endif
        return 0;
    }

    LOGPRINT_ERROR("Did not find fd=%u in cache.", (unsigned int)fd);
    return -1;
}

/******************************************************************************
    cache_add_fd
*//**
    @brief Gets a new file descriptor and adds to cache.
    @param[in] lfs  Pointer to the lfs filesystem.
    Returns the created CacheItem or NULL on failure.
******************************************************************************/
static CacheItem *
cache_add_fd(lfs_t *lfs)
{
    int fd;
    CacheItem *item;
    lfs_descriptor_t *descr = NULL;

    fd = lfs_pool_get_descriptor(pool, &descr);
    if (fd < 0)
    {
        return NULL;
    }

    /* Got a descriptor, now add to cache. */
    item = (CacheItem *)malloc(sizeof(CacheItem));
    if (!item)
    {
        LOGPRINT_ERROR("Error allocating memory.");
        lfs_pool_put_descriptor(pool, (uint8_t)fd);
        return NULL;
    }

    item->fd = fd;
    item->descr = descr;
    item->lfs = lfs;
    CList_append(&cache, item);
    LOGPRINT_DEBUG("Added fd=%u to cache.", (unsigned int)fd);
    return item;
}

/******************************************************************************
    get_lfs
*//**
    @brief Gets filesystem objects from partition.
    @param[in] label  Parition label.
    @param[in] lpfs  Returned Lfs_Part_t object.
    @param[out] lfs  Returned lfs filesystem.
    @return Returns 0 on success, -1 on error.
******************************************************************************/
static int
get_lfs(const char *label, Lfs_Part_t **lpfs, lfs_t **lfs)
{
    Lfs_Part_t *lp;
    lp = Lfs_Part_getPartition(label);
    if (!lp)
    {
        LOGPRINT_ERROR("Cound not get partition with label: %s", label);
        return -1;
    }

    *lpfs = lp;
    *lfs = &lp->lfs;
    return 0;
}

/******************************************************************************
    getfsinfo

    Call params:
        call->part_label: string 
    Reply params:
        reply->address: uint32 
        reply->size: uint32 
        reply->block_size: uint32 
        reply->block_count: uint32 
*//**
    @brief Implements the RPC getfsinfo handler.
******************************************************************************/
static void
getfsinfo(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lfspart_LfsCallset *call_msg = (lfspart_LfsCallset *)call_frame;
    lfspart_LfsCallset *reply_msg = (lfspart_LfsCallset *)reply_frame;
    lfspart_GetFsInfo_call *call = &call_msg->msg.getfsinfo_call;
    lfspart_GetFsInfo_reply *reply = &reply_msg->msg.getfsinfo_reply;
    Lfs_Part_t *lpfs;
    lfs_t *lfs;
    struct lfs_fsinfo fsinfo;
    int ret;

    LOGPRINT_DEBUG("In getfsinfo handler");

    reply_msg->which_msg = lfspart_LfsCallset_getfsinfo_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    memset(reply, 0, sizeof(lfspart_GetFsInfo_reply));

    ret = get_lfs(call->part_label, &lpfs, &lfs);
    if (ret < 0)
    {
        LOGPRINT_ERROR("Cound not get partition with label: %s", call->part_label);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    ret = lfs_fs_stat(lfs, &fsinfo);
    if (ret < 0)
    {
        LOGPRINT_ERROR("Cound not get fs info: %d", ret);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    reply->address = lpfs->partition->address;
    reply->size = lpfs->partition->size;
    reply->block_size = fsinfo.block_size;
    reply->block_count = fsinfo.block_count;
    reply->block_count = fsinfo.block_count;
}

/******************************************************************************
    diropen

    Call params:
        call->part_label: string 
        call->path: string 
    Reply params:
        reply->fd: int32 
*//**
    @brief Implements the RPC diropen handler.
******************************************************************************/
static void
diropen(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lfspart_LfsCallset *call_msg = (lfspart_LfsCallset *)call_frame;
    lfspart_LfsCallset *reply_msg = (lfspart_LfsCallset *)reply_frame;
    lfspart_DirOpen_call *call = &call_msg->msg.diropen_call;
    lfspart_DirOpen_reply *reply = &reply_msg->msg.diropen_reply;
    Lfs_Part_t *lpfs;
    lfs_t *lfs;
    struct lfs_info info;
    CacheItem *item;
    int ret;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("In diropen handler");

    reply_msg->which_msg = lfspart_LfsCallset_diropen_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    ret = get_lfs(call->part_label, &lpfs, &lfs);
    if (ret < 0)
    {
        LOGPRINT_ERROR("Cound not get partition with label: %s", call->part_label);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    if (lfs_exists(lfs, call->path, &info) < 0)
    {
        LOGPRINT_ERROR("Path does not exist: %s", call->path);
        reply->fd = -1;
        return;
    }

    item = cache_add_fd(lfs);
    if (!item)
    {
        reply->fd = -1;
        return;
    }

    /* Copy info to cache item's info field. */
    memcpy(&item->info, &info, sizeof(struct lfs_info));

    reply->fd = item->fd;

    ret = lfs_dir_open(lfs, &item->descr->dir, call->path);
    if (ret < 0)
    {
        cache_rm_fd(item->fd);
        reply->fd = -1;
        LOGPRINT_ERROR("Failed open dir %s", call->path);
        return;
    }

    LOGPRINT_DEBUG("Directory %s is open, using fd=%u",
        call->path, (unsigned int)item->fd);
}

/******************************************************************************
    dirclose

    Call params:
        call->fd: uint32 
    Reply params:
*//**
    @brief Implements the RPC dirclose handler.
******************************************************************************/
static void
dirclose(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lfspart_LfsCallset *call_msg = (lfspart_LfsCallset *)call_frame;
    lfspart_LfsCallset *reply_msg = (lfspart_LfsCallset *)reply_frame;
    lfspart_DirClose_call *call = &call_msg->msg.dirclose_call;
    lfspart_DirClose_reply *reply = &reply_msg->msg.dirclose_reply;
    CacheItem *item;
    int ret;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("In dirclose handler");

    reply_msg->which_msg = lfspart_LfsCallset_dirclose_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    item = cache_find_fd(call->fd);
    if (!item)
    {
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    ret = lfs_dir_close(item->lfs, &item->descr->dir);
    if (ret < 0)
    {
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    LOGPRINT_DEBUG("Directory %s is now closed.", item->info.name);
    ret = cache_rm_fd(item->fd);
    if (ret < 0)
    {
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }
}

/******************************************************************************
    dirread

    Call params:
        call->fd: uint32 
    Reply params:
        reply->valid: bool 
        reply->info: message 
*//**
    @brief Implements the RPC dirread handler.
******************************************************************************/
static void
dirread(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lfspart_LfsCallset *call_msg = (lfspart_LfsCallset *)call_frame;
    lfspart_LfsCallset *reply_msg = (lfspart_LfsCallset *)reply_frame;
    lfspart_DirRead_call *call = &call_msg->msg.dirread_call;
    lfspart_DirRead_reply *reply = &reply_msg->msg.dirread_reply;
    uint32_t name_max = PROTORPC_ARRAY_LENGTH(reply->info.name);
    CacheItem *item;
    struct lfs_info info;
    int ret;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("In dirread handler max=%u", (unsigned int)name_max);

    reply_msg->which_msg = lfspart_LfsCallset_dirread_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    /* Lookup item in cache by provided fd. */
    item = cache_find_fd(call->fd);
    if (!item)
    {
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    ret = lfs_dir_read(item->lfs, &item->descr->dir, &info);
    if (ret < 0)
    {
        *status = StatusEnum_RPC_HANDLER_ERROR;
        reply->valid = 0;
        return;
    }

    reply->valid = 1;
    reply->info.type = info.type;
    reply->info.size = info.size;
    strncpy(reply->info.name, info.name, name_max); 
}

/******************************************************************************
    dirlist

    Call params:
        call->part_label: string 
        call->path: string 
        call->start_idx: uint32 
    Reply params:
        reply->valid: bool 
        reply->num_entries: uint32 
        reply->start_idx: uint32 
        reply->info_array: message [repeated]
*//**
    @brief Implements the RPC dirlist handler.
******************************************************************************/
static void
dirlist(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lfspart_LfsCallset *call_msg = (lfspart_LfsCallset *)call_frame;
    lfspart_LfsCallset *reply_msg = (lfspart_LfsCallset *)reply_frame;
    lfspart_DirList_call *call = &call_msg->msg.dirlist_call;
    lfspart_DirList_reply *reply = &reply_msg->msg.dirlist_reply;
    uint32_t name_max = PROTORPC_ARRAY_LENGTH(reply->info_array[0].name);
    uint32_t entries_max = PROTORPC_ARRAY_LENGTH(reply->info_array);
    Lfs_Part_t *lpfs;
    lfs_t *lfs;
    lfs_dir_t dir;
    struct lfs_info info;
    int ret;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("In dirlist handler (%u, %u)",
        (unsigned int)name_max, (unsigned int)entries_max);

    reply_msg->which_msg = lfspart_LfsCallset_dirlist_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    if ((ret = get_lfs(call->part_label, &lpfs, &lfs)) < 0)
    {
        LOGPRINT_ERROR("Cound not get partition with label: %s", call->part_label);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    if (lfs_exists(lfs, call->path, NULL) < 0)
    {
        reply->valid = false;
        reply->num_entries = 0;
        reply->info_array_count = 0; 
        LOGPRINT_ERROR("Dir does not exist: %s", call->path);
        return;
    }

    if ((ret = lfs_dir_open(lfs, &dir, call->path)) < 0)
    {
        reply->valid = false;
        reply->num_entries = 0;
        reply->info_array_count = 0; 
        LOGPRINT_ERROR("Failed open dir %s: %d", call->path, ret);
        return;
    }

    uint32_t entry_idx = 0;
    uint32_t num_returned = 0;
    while (1)
    {
        /* lfs_dir_read returns 0 at end of directory. */
        ret = lfs_dir_read(lfs, &dir, &info);
        if (ret > 0)
        {
            if ((call->start_idx <= entry_idx) && (num_returned < entries_max))
            {
                reply->info_array[num_returned].type = info.type;
                reply->info_array[num_returned].size = info.size;
                strncpy(reply->info_array[num_returned++].name,
                        info.name,
                        name_max); 
                LOGPRINT_DEBUG("[%u]: %c %u: %s",
                    (unsigned int)entry_idx,
                    (info.type == LFS_TYPE_REG) ? 'f' : 'd',
                    (unsigned int)info.size,
                    info.name);
            }
            entry_idx++;
        }
        else 
        {
            if (ret < 0)
            {
                LOGPRINT_ERROR("Got error on dir read, closing dir: %d", ret);
            }
            else
            {
                LOGPRINT_DEBUG("Finished reading dir, closing.");
            }
            lfs_dir_close(lfs, &dir);
            break;
        }
    }

    reply->valid = true;
    reply->num_entries = entry_idx;
    reply->start_idx = call->start_idx;
    reply->info_array_count = num_returned; 
}

/******************************************************************************
    fileopen

    Call params:
        call->part_label: string 
        call->path: string 
        call->flags: uint32 
    Reply params:
        reply->status: int32 
        reply->fd: int32 
        reply->info: message 
*//**
    @brief Implements the RPC fileopen handler.
******************************************************************************/
static void
fileopen(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lfspart_LfsCallset *call_msg = (lfspart_LfsCallset *)call_frame;
    lfspart_LfsCallset *reply_msg = (lfspart_LfsCallset *)reply_frame;
    lfspart_FileOpen_call *call = &call_msg->msg.fileopen_call;
    lfspart_FileOpen_reply *reply = &reply_msg->msg.fileopen_reply;
    Lfs_Part_t *lpfs;
    lfs_t *lfs;
    CacheItem *item;
    int ret;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("==> In fileopen handler");

    reply_msg->which_msg = lfspart_LfsCallset_fileopen_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    ret = get_lfs(call->part_label, &lpfs, &lfs);
    if (ret < 0)
    {
        LOGPRINT_ERROR("Cound not get partition with label: %s", call->part_label);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    if (call->flags == 0)
    {
        LOGPRINT_ERROR("Null OPEN flags!");
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    item = cache_add_fd(lfs);
    if (!item)
    {
        LOGPRINT_ERROR("Could not get file descriptor");
        reply->status = 0;
        reply->fd = -1;
        return;
    }
    
    LOGPRINT_DEBUG("Opening %s with flags 0x%08x",
        call->path, (unsigned int)call->flags);

    ret = lfs_file_open(lfs, &item->descr->file, call->path, call->flags);
    if (ret < 0)
    {
        cache_rm_fd(item->fd);
        reply->status = ret;
        reply->fd = -1;
        LOGPRINT_ERROR("Failed file open (flags=0x%08x): %s (ret=%d)",
            (unsigned int)call->flags, call->path, ret);
        return;
    }

    /* Add info to the item. */
    ret = lfs_stat(lfs, call->path, &item->info);
    if (ret < 0)
    {
        LOGPRINT_ERROR("Failed to stat open file: %s (ret=%d)",
            call->path, ret);
    }

    reply->status = 0;
    reply->fd = item->fd;
    LOGPRINT_DEBUG("File %s is open with flags=0x%08x, using fd=%u",
        call->path, (unsigned int)call->flags, (unsigned int)item->fd);
}

/******************************************************************************
    fileclose

    Call params:
        call->fd: uint32 
    Reply params:
*//**
    @brief Implements the RPC fileclose handler.
******************************************************************************/
static void
fileclose(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lfspart_LfsCallset *call_msg = (lfspart_LfsCallset *)call_frame;
    lfspart_LfsCallset *reply_msg = (lfspart_LfsCallset *)reply_frame;
    lfspart_FileClose_call *call = &call_msg->msg.fileclose_call;
    lfspart_FileClose_reply *reply = &reply_msg->msg.fileclose_reply;
    CacheItem *item;
    int ret;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("==> In fileclose handler");

    reply_msg->which_msg = lfspart_LfsCallset_fileclose_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    item = cache_find_fd(call->fd);
    if (!item)
    {
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    ret = lfs_file_close(item->lfs, &item->descr->file);
    if (ret < 0)
    {
        LOGPRINT_ERROR("Error closing fd=%u: %d", (unsigned int)call->fd, ret);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    LOGPRINT_DEBUG("File %s is now closed.", item->info.name);

    ret = cache_rm_fd(item->fd);
    if (ret < 0)
    {
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }
}

/******************************************************************************
    fileread

    Call params:
        call->fd: uint32 
        call->offset: uint32 
        call->seek_flag: uint32 
        call->read_size: uint32 
    Reply params:
        reply->status: int32 
        reply->offset: uint32 
        reply->data: bytes 
*//**
    @brief Implements the RPC fileread handler.
******************************************************************************/
static void
fileread(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lfspart_LfsCallset *call_msg = (lfspart_LfsCallset *)call_frame;
    lfspart_LfsCallset *reply_msg = (lfspart_LfsCallset *)reply_frame;
    lfspart_FileRead_call *call = &call_msg->msg.fileread_call;
    lfspart_FileRead_reply *reply = &reply_msg->msg.fileread_reply;
    uint32_t size_max = PROTORPC_ARRAY_LENGTH(reply->data.bytes);
    uint32_t read_size;
    CacheItem *item;
    int ret;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("==> In fileread handler");

    reply_msg->which_msg = lfspart_LfsCallset_fileread_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    item = cache_find_fd(call->fd);
    if (!item)
    {
        LOGPRINT_ERROR("Could not retrieve file descriptor: %u",
            (unsigned int)call->fd);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    if ((call->seek_flag != LFS_SEEK_SET) &&
        (call->seek_flag != LFS_SEEK_CUR) &&
        (call->seek_flag != LFS_SEEK_END))
    {
        LOGPRINT_ERROR("Invalid seek flag: 0x%08x", (unsigned int)call->seek_flag);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    if (call->read_size > size_max)
    {
        LOGPRINT_ERROR("Read size too large: requested %u > %u",
            (unsigned int)call->read_size, (unsigned int)size_max);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
        
    }

    reply->offset = call->offset;
    read_size = MIN(call->read_size, size_max);

    ret = lfs_read_from_offset(item->lfs,
                               &item->descr->file,
                               call->offset,
                               call->seek_flag,
                               reply->data.bytes,
                               read_size);
    if (ret < 0)
    {
        LOGPRINT_ERROR("read returned  %d", ret);
        reply->data.size = 0;
    }
    else
    {
        LOGPRINT_DEBUG("Read file: %s size=%u; offset=%u (ret=%d).",
            item->info.name,
            (unsigned int)read_size,
            (unsigned int)call->offset,
            ret);

        reply->data.size = ret;
    }

    /* Returns the number of bytes read in data. */
    reply->status = ret;
    return;
}

/******************************************************************************
    filewrite

    Call params:
        call->fd: uint32 
        call->offset: uint32 
        call->seek_flag: uint32 
        call->data: bytes 
    Reply params:
        reply->status: int32 
*//**
    @brief Implements the RPC filewrite handler.
******************************************************************************/
static void
filewrite(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lfspart_LfsCallset *call_msg = (lfspart_LfsCallset *)call_frame;
    lfspart_LfsCallset *reply_msg = (lfspart_LfsCallset *)reply_frame;
    lfspart_FileWrite_call *call = &call_msg->msg.filewrite_call;
    lfspart_FileWrite_reply *reply = &reply_msg->msg.filewrite_reply;
    CacheItem *item;
    int ret;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("==> In filewrite handler");

    reply_msg->which_msg = lfspart_LfsCallset_filewrite_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    item = cache_find_fd(call->fd);
    if (!item)
    {
        LOGPRINT_ERROR("Could not retrieve file descriptor: %u",
            (unsigned int)call->fd);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    if ((call->seek_flag != LFS_SEEK_SET) &&
        (call->seek_flag != LFS_SEEK_CUR) &&
        (call->seek_flag != LFS_SEEK_END))
    {
        LOGPRINT_ERROR("Invalid seek flag: 0x%08x", (unsigned int)call->seek_flag);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    ret = lfs_write_to_offset(item->lfs,
                              &item->descr->file,
                              call->offset,
                              call->seek_flag,
                              call->data.bytes,
                              call->data.size);
    if (ret < 0)
    {
        LOGPRINT_ERROR("write returned  %d", ret);
    }

    LOGPRINT_DEBUG("Wrote %u bytes to file %s to offset %u.",
        (unsigned int)call->data.size,
        item->info.name,
        (unsigned int)call->offset);

    /* Returns the number of bytes written. */
    reply->status = ret;
    return;
}

/******************************************************************************
    remove

    Call params:
        call->part_label: string 
        call->path: string 
    Reply params:
        reply->status: int32 
*//**
    @brief Implements the RPC remove handler.
******************************************************************************/
static void
remove_path(void *call_frame, void *reply_frame, StatusEnum *status)
{
    lfspart_LfsCallset *call_msg = (lfspart_LfsCallset *)call_frame;
    lfspart_LfsCallset *reply_msg = (lfspart_LfsCallset *)reply_frame;
    lfspart_Remove_call *call = &call_msg->msg.remove_call;
    lfspart_Remove_reply *reply = &reply_msg->msg.remove_reply;
    Lfs_Part_t *lpfs;
    lfs_t *lfs;
    int ret;

    (void)call;
    (void)reply;

    LOGPRINT_DEBUG("In remove handler");

    reply_msg->which_msg = lfspart_LfsCallset_remove_reply_tag;
    *status = StatusEnum_RPC_SUCCESS;

    ret = get_lfs(call->part_label, &lpfs, &lfs);
    if (ret < 0)
    {
        LOGPRINT_ERROR("Cound not get partition with label: %s", call->part_label);
        *status = StatusEnum_RPC_HANDLER_ERROR;
        return;
    }

    LOGPRINT_DEBUG("Removing file %s.", call->path);
    ret = lfs_remove(lfs, call->path);
    if (ret < 0)
    {
        LOGPRINT_ERROR("Error removing path: %s", call->path);
    }
    reply->status = ret;
}

static ProtoRpc_Handler_Entry handlers[] = {
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_getfsinfo_call_tag, getfsinfo),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_diropen_call_tag, diropen),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_dirclose_call_tag, dirclose),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_dirread_call_tag, dirread),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_dirlist_call_tag, dirlist),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_fileopen_call_tag, fileopen),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_fileclose_call_tag, fileclose),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_fileread_call_tag, fileread),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_filewrite_call_tag, filewrite),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_remove_call_tag, remove_path),
};

#define NUM_HANDLERS    PROTORPC_ARRAY_LENGTH(handlers)

/******************************************************************************
    [docimport Lfs_PartRpc_init]
*//**
    @brief Initialization for the Lfs_PartRpc.
    @return Returns 0 on success, -1 on failure.
******************************************************************************/
int
Lfs_PartRpc_init(Lfs_Part_t *lpfs)
{
    CList_init(&cache);

    _lpfs = lpfs;
    pool = lfs_pool_init(MAX_OPEN_DESCRIPTORS);
    if (!pool)
    {
        _lpfs = NULL;
        return -1;
    }
    return 0;
}

/******************************************************************************
    [docimport Lfs_PartRpc_resolver]
*//**
    @brief Resolver function for lfspart_rpc.
    @param[in] call_frame  Pointer to the unpacked call frame object.
    @param[in] offset  Offset of the callset member within the call_frame.
******************************************************************************/
ProtoRpc_handler *
Lfs_PartRpc_resolver(void *call_frame, uint32_t offset)
{
    uint8_t *frame = (uint8_t *)call_frame;
    lfspart_LfsCallset *this = (lfspart_LfsCallset *)&frame[offset];
    unsigned int i;

    /** @brief Handler lookup */
    for (i = 0; i < NUM_HANDLERS; i++)
    {
        ProtoRpc_Handler_Entry *entry = &handlers[i];
        if (entry->tag == this->which_msg)
        {
            return entry->handler;
        }
    }

    return NULL;
}
