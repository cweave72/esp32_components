/*******************************************************************************
 *  @file: lfspart_rpc.c
 *
 *  @brief: Handlers for lfspart_rpc.
*******************************************************************************/
#include "CList.h"
#include "Lfs_Part.h"
#include "Lfs_PartRpc.pb.h"
#include "ProtoRpc.h"
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
    /** @brief item path */
    char name[LFS_NAME_MAX];

} CacheItem;

/** @brief The cache holding open fd's. */
static CList cache;

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
        LOGPRINT_DEBUG("- %u: %s", (unsigned int)item->fd, item->name);
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

    fd = lfs_pool_get_descriptor(pool, descr);
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
#ifdef LOCAL_DEBUG
    cache_print();
#endif
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

    if (lfs_exists(lfs, call->path) < 0)
    {
        reply->fd = -1;
        return;
    }

    item = cache_add_fd(lfs);
    if (!item)
    {
        reply->fd = -1;
        return;
    }
    
    reply->fd = item->fd;

    ret = lfs_dir_open(lfs, &item->descr->dir, call->path);
    if (ret < 0)
    {
        cache_rm_fd(item->fd);
        reply->fd = -1;
        LOGPRINT_ERROR("Failed open dir %s", call->path);
        return;
    }

    strcpy(item->name, call->path);
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

    LOGPRINT_DEBUG("Directory %s is now closed.", item->name);
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

    if (lfs_exists(lfs, call->path) < 0)
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


static ProtoRpc_Handler_Entry handlers[] = {
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_getfsinfo_call_tag, getfsinfo),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_diropen_call_tag, diropen),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_dirclose_call_tag, dirclose),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_dirread_call_tag, dirread),
    PROTORPC_ADD_HANDLER(lfspart_LfsCallset_dirlist_call_tag, dirlist),
};

#define NUM_HANDLERS    PROTORPC_ARRAY_LENGTH(handlers)

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
