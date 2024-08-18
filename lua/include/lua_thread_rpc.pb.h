/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.8-dev */

#ifndef PB_LUA_LUA_THREAD_RPC_PB_H_INCLUDED
#define PB_LUA_LUA_THREAD_RPC_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
/* Runs a script file. */
typedef struct _lua_RunScript_call {
    char filename[64];
} lua_RunScript_call;

typedef struct _lua_RunScript_reply {
    /* Script run status */
    int32_t status;
} lua_RunScript_reply;

/* Gets any error message or traceback from the last RunScript call. */
typedef struct _lua_GetLastMessage_call {
    char dummy_field;
} lua_GetLastMessage_call;

typedef struct _lua_GetLastMessage_reply {
    /* Any error message */
    char msg[900];
} lua_GetLastMessage_reply;

typedef struct _lua_LuaCallset {
    pb_size_t which_msg;
    union {
        lua_RunScript_call runScript_call;
        lua_RunScript_reply runScript_reply;
        lua_GetLastMessage_call getLastMessage_call;
        lua_GetLastMessage_reply getLastMessage_reply;
    } msg;
} lua_LuaCallset;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define lua_RunScript_call_init_default          {""}
#define lua_RunScript_reply_init_default         {0}
#define lua_GetLastMessage_call_init_default     {0}
#define lua_GetLastMessage_reply_init_default    {""}
#define lua_LuaCallset_init_default              {0, {lua_RunScript_call_init_default}}
#define lua_RunScript_call_init_zero             {""}
#define lua_RunScript_reply_init_zero            {0}
#define lua_GetLastMessage_call_init_zero        {0}
#define lua_GetLastMessage_reply_init_zero       {""}
#define lua_LuaCallset_init_zero                 {0, {lua_RunScript_call_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define lua_RunScript_call_filename_tag          1
#define lua_RunScript_reply_status_tag           1
#define lua_GetLastMessage_reply_msg_tag         1
#define lua_LuaCallset_runScript_call_tag        1
#define lua_LuaCallset_runScript_reply_tag       2
#define lua_LuaCallset_getLastMessage_call_tag   3
#define lua_LuaCallset_getLastMessage_reply_tag  4

/* Struct field encoding specification for nanopb */
#define lua_RunScript_call_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   filename,          1)
#define lua_RunScript_call_CALLBACK NULL
#define lua_RunScript_call_DEFAULT NULL

#define lua_RunScript_reply_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    status,            1)
#define lua_RunScript_reply_CALLBACK NULL
#define lua_RunScript_reply_DEFAULT NULL

#define lua_GetLastMessage_call_FIELDLIST(X, a) \

#define lua_GetLastMessage_call_CALLBACK NULL
#define lua_GetLastMessage_call_DEFAULT NULL

#define lua_GetLastMessage_reply_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   msg,               1)
#define lua_GetLastMessage_reply_CALLBACK NULL
#define lua_GetLastMessage_reply_DEFAULT NULL

#define lua_LuaCallset_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MESSAGE,  (msg,runScript_call,msg.runScript_call),   1) \
X(a, STATIC,   ONEOF,    MESSAGE,  (msg,runScript_reply,msg.runScript_reply),   2) \
X(a, STATIC,   ONEOF,    MESSAGE,  (msg,getLastMessage_call,msg.getLastMessage_call),   3) \
X(a, STATIC,   ONEOF,    MESSAGE,  (msg,getLastMessage_reply,msg.getLastMessage_reply),   4)
#define lua_LuaCallset_CALLBACK NULL
#define lua_LuaCallset_DEFAULT NULL
#define lua_LuaCallset_msg_runScript_call_MSGTYPE lua_RunScript_call
#define lua_LuaCallset_msg_runScript_reply_MSGTYPE lua_RunScript_reply
#define lua_LuaCallset_msg_getLastMessage_call_MSGTYPE lua_GetLastMessage_call
#define lua_LuaCallset_msg_getLastMessage_reply_MSGTYPE lua_GetLastMessage_reply

extern const pb_msgdesc_t lua_RunScript_call_msg;
extern const pb_msgdesc_t lua_RunScript_reply_msg;
extern const pb_msgdesc_t lua_GetLastMessage_call_msg;
extern const pb_msgdesc_t lua_GetLastMessage_reply_msg;
extern const pb_msgdesc_t lua_LuaCallset_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define lua_RunScript_call_fields &lua_RunScript_call_msg
#define lua_RunScript_reply_fields &lua_RunScript_reply_msg
#define lua_GetLastMessage_call_fields &lua_GetLastMessage_call_msg
#define lua_GetLastMessage_reply_fields &lua_GetLastMessage_reply_msg
#define lua_LuaCallset_fields &lua_LuaCallset_msg

/* Maximum encoded size of messages (where known) */
#define lua_GetLastMessage_call_size             0
#define lua_GetLastMessage_reply_size            902
#define lua_LuaCallset_size                      905
#define lua_RunScript_call_size                  65
#define lua_RunScript_reply_size                 11

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif