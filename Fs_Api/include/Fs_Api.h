/*******************************************************************************
 *  @file: Fs_Api.h
 *   
 *  @brief: Generic API for accessing an filesystem.
*******************************************************************************/
#ifndef FS_API_H
#define FS_API_H

typedef enum {
    FSAPI_O_RDONLY = 0x1 << 1,           // Open a file as read only
    FSAPI_O_WRONLY = 0x1 << 2,           // Open a file as write only
    FSAPI_O_RDWR   = 0x1 << 3,           // Open a file as read and write
    FSAPI_O_CREAT  = 0x1 << 4,           // Create a file if it does not exist
    FSAPI_O_EXCL   = 0x1 << 5,           // Fail if a file already exists
    FSAPI_O_TRUNC  = 0x1 << 6,           // Truncate the existing file to zero size
    FSAPI_O_APPEND = 0x1 << 7,           // Move to end of file on every write
} Fs_Api_open_flags;

typedef enum {
    FSAPI_SEEK_SET,   // Seek relative to an absolute position
    FSAPI_SEEK_CUR,   // Seek relative to the current file position
    FSAPI_SEEK_END,   // Seek relative to the end of the file
} Fs_Api_seek_flags;

#define FS_API_CONTENTS()                                 \
    /** @brief Opaque context object. */                  \
    void *ctx;                                            \
                                                          \
    /** @brief File operations. */                        \
    int (*open)(void *ctx, const char *path, int flags);  \
    int (*close)(void *ctx, int fd);                      \
    int (*read)(void *ctx, int fd, char *buf, int size);  \
    int (*write)(void *ctx, int fd, char *buf, int size); \
    int (*seek)(void *ctx, int fd, int offset, int mode); \
    int (*fsize)(void *ctx, int fd);

/** @brief Fs_Api base object.
*/
typedef struct Fs_Api
{
    FS_API_CONTENTS()
} Fs_Api;
#endif

