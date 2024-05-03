/*******************************************************************************
 *  @file: Lfs_Part.c
 *  
 *  @brief: A littlefs wrapper using the esp_parition api.
*******************************************************************************/
#include <string.h>
#include <stdbool.h>
#include "Lfs_Part.h"
#include "CheckCond.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "Lfs_Part";

/** @brief List of registered partitions. */
static CList registry;
static bool registry_initialized = false;

/******************************************************************************
    part_read
*//**
    @brief Partition read API function.
    @param[in] c  Pointer to lfs configuration object.
    @param[in] block  lfs block.
    @param[in] off  lfs offset.
    @param[in] buffer  Pointer to read buffer.
    @param[in] size  Size of buffer.
    @return Returns 0 on success, negative error code.
******************************************************************************/
static int
part_read(
    const struct lfs_config *c,
    lfs_block_t block,
    lfs_off_t off,
    void *buffer,
    lfs_size_t size)
{
    Lfs_Part_t *lpfs = (Lfs_Part_t *)c->context;
    size_t part_off = (block * c->block_size) + off;
    esp_err_t err;

    LOGPRINT_VERBOSE("read block %u, 0x%08x %u bytes",
        (unsigned int)block,
        (unsigned int)part_off,
        (unsigned int)size);
    err = esp_partition_read(lpfs->partition, part_off, buffer, size);
    if (err)
    {
        LOGPRINT_ERROR("Failed to read addr %08x, size %08x, err %d",
            (unsigned int) part_off,
            (unsigned int) size,
            err);
        return LFS_ERR_IO;
    }
    return 0;
}

/******************************************************************************
    part_write
*//**
    @brief Partition write API function.
    @param[in] c  Pointer to lfs configuration object.
    @param[in] block  lfs block.
    @param[in] off  lfs offset.
    @param[in] buffer  Pointer to buffer to write.
    @param[in] size  Size of buffer.
    @return Returns 0 on success, negative error code.
******************************************************************************/
static int
part_write(
    const struct lfs_config *c,
    lfs_block_t block,
    lfs_off_t off,
    const void *buffer,
    lfs_size_t size)
{
    Lfs_Part_t *lpfs = c->context;
    size_t part_off = (block * c->block_size) + off;
    esp_err_t err;

    LOGPRINT_VERBOSE("write block %u, 0x%08x %u bytes",
        (unsigned int)block,
        (unsigned int)part_off,
        (unsigned int)size);
    err = esp_partition_write(lpfs->partition, part_off, buffer, size);
    if (err)
    {
        LOGPRINT_ERROR("Failed to write addr %08x, size %08x, err %d",
            (unsigned int) part_off,
            (unsigned int) size,
            err);
        return LFS_ERR_IO;
    }
    return 0;
}

/******************************************************************************
    part_erase
*//**
    @brief Partition erase API function.
    @param[in] c  Pointer to lfs configuration object.
    @param[in] block  lfs block.
    @return Returns 0 on success, negative error code.
******************************************************************************/
static int
part_erase(const struct lfs_config *c, lfs_block_t block)
{
    Lfs_Part_t *lpfs = c->context;
    size_t part_off = block * c->block_size;
    esp_err_t err;

    LOGPRINT_VERBOSE("erase block %u, 0x%08x",
        (unsigned int)block,
        (unsigned int)part_off);
    err = esp_partition_erase_range(lpfs->partition, part_off, c->block_size);
    if (err)
    {
        LOGPRINT_ERROR("Failed to erase addr %08x, size %08x, err %d",
            (unsigned int)part_off,
            (unsigned int)c->block_size,
            err);
        return LFS_ERR_IO;
    }
    return 0;
}

/******************************************************************************
    part_sync
*//**
    @brief Partition sync API function.
    @param[in] c  Pointer to lfs configuration object.
    @return Returns 0 on success, negative error code.
******************************************************************************/
static int
part_sync(const struct lfs_config *c)
{
    /* Unnecessary for esp-idf */
    return 0;
}

static int
lock(const struct lfs_config *c)
{
    Lfs_Part_t *lpfs = c->context;
    LOGPRINT_VERBOSE("getting lock.");
    RTOS_MUTEX_GET(lpfs->lock);
    return 0;
}

static int
unlock(const struct lfs_config *c)
{
    Lfs_Part_t *lpfs = c->context;
    LOGPRINT_VERBOSE("releasing lock");
    RTOS_MUTEX_PUT(lpfs->lock);
    return 0;
}

/******************************************************************************
    [docimport Lfs_Part_register]
*//**
    @brief Registers a littlefs partition.
    Call this after Lfs_Part_init().
    @param[in] lpfs  Pointer to Lfs_Part_t object instance.
******************************************************************************/
void
Lfs_Part_register(Lfs_Part_t *lpfs)
{
    if (!registry_initialized) {
        CList_init(&registry);
        registry_initialized = true;
    }

    CList_append(&registry, lpfs);
    LOGPRINT_INFO("Registered littlefs partition: %s", lpfs->partition->label);
}

/******************************************************************************
    [docimport Lfs_Part_getPartition]
*//**
    @brief Gets a partition object from the registry.
    @param[in] part_label  Label for partition to get.
    @return Returns the matching Lfs_Part_t object, NULL otherwise.
******************************************************************************/
Lfs_Part_t *
Lfs_Part_getPartition(const char *label)
{
    Lfs_Part_t *entry;

    CLIST_ITER_ENTRY(entry, &registry)
    {
        if (strcmp(entry->partition->label, label) == 0)
        {
            LOGPRINT_DEBUG("Found registry with label: %s", label);
            return entry;
        }
    }
    LOGPRINT_ERROR("Failed to find matching partition: %s", label);
    return NULL;
}

/******************************************************************************
    [docimport Lfs_Part_init]
*//**
    @brief Initializes an Lfs_Part file system.
    @param[in] lpfs  Pointer to Lfs_Part_t object instance.
    @param[in] part_label  Label for partition to use.
    @param[out] lfs_result  lfs operation status.
******************************************************************************/
esp_err_t
Lfs_Part_init(Lfs_Part_t *lpfs, const char *part_label, int *lfs_result)
{
    int res;
    const esp_partition_t *part;

    *lfs_result = LFS_ERR_OK;

    /** @brief Get flash partition. */
    part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        ESP_PARTITION_SUBTYPE_ANY,
        part_label);
    CHECK_COND_RETURN_MSG(!part, ESP_ERR_NOT_FOUND, "Could not find partition.");

    lpfs->lock = RTOS_MUTEX_CREATE_STATIC(&lpfs->lockbuf);
    CHECK_COND_RETURN_MSG(!lpfs->lock, ESP_FAIL, "Failed creating mutex.");

    lpfs->partition   = part;
    lpfs->cfg.context = lpfs;
    lpfs->cfg.lock    = lock;
    lpfs->cfg.unlock  = unlock;

    /* Block device operations. */
    lpfs->cfg.read  = part_read;
    lpfs->cfg.prog  = part_write;
    lpfs->cfg.erase = part_erase;
    lpfs->cfg.sync  = part_sync;
    
    lpfs->cfg.read_size      = LFS_PART_READ_SIZE;
    lpfs->cfg.prog_size      = LFS_PART_WRITE_SIZE;
    lpfs->cfg.cache_size     = LFS_PART_CACHE_SIZE;
    lpfs->cfg.block_size     = LFS_PART_BLOCK_SIZE;
    lpfs->cfg.block_cycles   = LFS_PART_BLOCK_CYCLES;
    lpfs->cfg.lookahead_size = LFS_PART_LOOKAHEAD_SIZE;
    lpfs->cfg.block_count    = part->size / LFS_PART_BLOCK_SIZE;

    /** @brief Attempt to mount filesystem. Format the partition on fail. */
    res = lfs_mount(&lpfs->lfs, &lpfs->cfg);
    if (res != LFS_ERR_OK)
    {
        LOGPRINT_INFO("Mount failed (err=%d)", res);
        LOGPRINT_INFO("Formatting partition at 0x%08x",
            (unsigned int)part->address);

        res = lfs_format(&lpfs->lfs, &lpfs->cfg);
        if (res != LFS_ERR_OK)
        {
            LOGPRINT_ERROR("Partition format failed (err=%d).", res);
            *lfs_result = res;
            return ESP_FAIL;
        }
        LOGPRINT_INFO("Format successful.");

        /* Re-attempt the mount on the now-formatted partition. */
        res = lfs_mount(&lpfs->lfs, &lpfs->cfg);
        if (res != LFS_ERR_OK)
        {
            LOGPRINT_INFO("Mount failed (err=%d)", res);
            *lfs_result = res;
            return ESP_FAIL;
        }
    }

    LOGPRINT_INFO("Mount successful.");

    return ESP_OK;
}
