/*******************************************************************************
 *  @file: lua_thread.c
 *  
 *  @brief: Component for executing Lua scripts in a thread.
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "lua_thread.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "lua_thread";

#define open(fs, pth, flgs)     (fs)->open((fs)->ctx, (pth), (flgs))
#define close(fs, fd)           (fs)->close((fs)->ctx, (fd))
#define read(fs, fd, buf, size) (fs)->read((fs)->ctx, (fd), (buf), (size))
#define filesize(fs, fd)        (fs)->fsize((fs)->ctx, (fd))

/******************************************************************************
    msghandler
*//**
    @brief Message handler used to run all chunks
******************************************************************************/
static int
msghandler(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);

    if (msg == NULL)
    {
        /* If the error object has a string method. */
        if (luaL_callmeta(L, 1, "__tostring") &&
            lua_type(L, -1) == LUA_TSTRING)
        {
            return 1;  /* that is the message */
        }
        else
        {
            msg = lua_pushfstring(L, "(error object is a %s value)",
                luaL_typename(L, 1));
        }
    }

    luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
    return 1;  /* return the traceback */
}

/******************************************************************************
    print_message
*//**
    @brief Prints an error message, adding the program name in front of it (if
    present).
    @param[in] pname  Program name.
    @param[in] msg  Message to print.
******************************************************************************/
static void
print_message(const char *pname, const char *msg)
{
    if (pname) {   printf("%s: ", pname); }
    printf("%s\n", msg);
}

/******************************************************************************
    report
*//**
    @brief Check whether 'status' is not OK and, if so, prints the error
    message on the top of the stack.
******************************************************************************/
static int
report(lua_State *L, int status)
{
    const char *msg;
    if (status != LUA_OK)
    {
        /* Get the message at the top of the stack. */
        msg = lua_tostring(L, -1);
        if (msg == NULL) { msg = "(error message not a string)"; }
        print_message("lua", msg);
        /* Remove the message. */
        lua_pop(L, 1);
    }
    return status;
}

/******************************************************************************
    exec_pcall
*//**
    @brief Interface to 'lua_pcall', which sets appropriate message function.
    Used to run all chunks.
******************************************************************************/
static int
exec_pcall(lua_State *L, int narg, int nres)
{
    int status;
    /* Get base of the stack. */
    int base = lua_gettop(L) - narg;
    /* Push message handler onto the stack. */
    lua_pushcfunction(L, msghandler);
    /* Insert below function and args. */
    lua_insert(L, base);
    status = lua_pcall(L, narg, nres, base);
    /* Remove message handler from stack. */
    lua_remove(L, base);
    return status;
}

/******************************************************************************
    exec_chunk
*//**
    @brief Executes the loaded chunk and prints the report.
******************************************************************************/
static int
exec_chunk(lua_State *L, int status)
{
    if (status == LUA_OK)
    {
        status = exec_pcall(L, 0, 0);
    }
    return report(L, status);
}

/******************************************************************************
    exec_file
*//**
    @brief Loads the provided script file as a string into the vm buffer and
    executes the chunk.
******************************************************************************/
static int
exec_file(lua_State *L, const char *s, int size)
{
    return exec_chunk(L, luaL_loadbuffer(L, s, size, "=(string)"));
}

/******************************************************************************
    lua_thread_main
*//**
    @brief Main task loop for executing lua scripts.
******************************************************************************/
static void
lua_thread_main(void *p)
{
    Lua_Thread *t = (Lua_Thread *)p;
    Fs_Api *fs = t->fs;
    esp_err_t ret;

    while (1)
    {
        BaseType_t qret;
        int fd, fsize, nread; 
        Lua_Thread_sQueueItem item;
        lua_State *L;

        /*  Wait here for a script file to be provided. */
        LOGPRINT_DEBUG("Waiting for script.");
        qret = RTOS_QUEUE_RECV(t->squeue, &item);
        if (qret != pdTRUE)
        {
            LOGPRINT_ERROR("Queue returned %d.", qret);
            continue;
        }

        LOGPRINT_DEBUG("Received file %s to execute.", item.script_file);
        fd = open(fs, item.script_file, FSAPI_O_RDONLY);
        if (fd < 0)
        {
            LOGPRINT_ERROR("Error opening file.");
            continue;
        }

        /* Allocate file size for reading script. */
        fsize = filesize(fs, fd);
        char *script = (char *)malloc(fsize);
        if (!script)
        {
            LOGPRINT_ERROR("Error allocating space for file read.");
            goto cleanup_2;
        }

        nread = read(fs, fd, script, fsize);
        if (nread <= 0)
        {
            goto cleanup_1;
        }

        LOGPRINT_DEBUG("Read %d bytes from file.", nread);

        /* Execute. */
        L = luaL_newstate();
        luaL_openlibs(L);

        ret = exec_file(L, script, nread);
        if (ret != LUA_OK)
        {
            LOGPRINT_ERROR("Error (exec_file): ret=%d\n", ret);
            goto cleanup_0;
        }
        /* Remove code from the stack after success. */
        lua_pop(L, lua_gettop(L));

cleanup_0:
        LOGPRINT_DEBUG("Closing lua vm.");
        lua_close(L);
cleanup_1:
        LOGPRINT_DEBUG("Freeing file buffer.");
        free(script);
cleanup_2:
        LOGPRINT_DEBUG("Closing script file buffer.");
        close(fs, fd);
    }
}

/******************************************************************************
    [docimport Lua_Thread_send]
*//**
    @brief Sends a script to be executed to the queue.
    @param[in] self to Lua_Thread object.
    @param[in] filename  Filename of script.
******************************************************************************/
int
Lua_Thread_send(Lua_Thread *self, const char *filename)
{
    Lua_Thread_sQueueItem item;
    BaseType_t ret;

    strcpy(item.script_file, filename);
    ret = RTOS_QUEUE_SEND_WAIT(self->squeue, &item, 100);
    if (ret != pdTRUE)
    {
        LOGPRINT_ERROR("Error on queue send (%d)", (unsigned int)ret);
        return -1;
    }

    return 0;
}

/******************************************************************************
    [docimport Lua_Thread_init]
*//**
    @brief Initializes a thread for executing Lua scripts.
    @param[in] t  Pointer to the Lua_Thread object.
******************************************************************************/
esp_err_t
Lua_Thread_init(Lua_Thread *t)
{
    esp_err_t ret;

    /* Must create the event group here prior to using it in the task. */
    t->squeue = RTOS_QUEUE_CREATE(2, sizeof(Lua_Thread_sQueueItem));

    ret = RTOS_TASK_CREATE(
        lua_thread_main,
        t->taskName,
        t->taskStackSize,
        (void *)t,
        t->taskPrio,
        &t->taskHandle);
    if (ret != ESP_OK)
    {
        LOGPRINT_ERROR("Failed creating Lua Thread task (%d)", ret);
        return ret;
    }

    return ESP_OK;
}
