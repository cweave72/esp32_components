/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.8-dev */

#ifndef PB_TEST_TESTRPC_PB_H_INCLUDED
#define PB_TEST_TESTRPC_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _test_Add_call {
    int32_t a;
    int32_t b;
} test_Add_call;

typedef struct _test_Add_reply {
    int32_t sum;
} test_Add_reply;

typedef struct _test_HandlerError_call {
    char dummy_field;
} test_HandlerError_call;

typedef struct _test_HandlerError_reply {
    char dummy_field;
} test_HandlerError_reply;

typedef PB_BYTES_ARRAY_T(16) test_SetStruct_call_var_bytes_t;
typedef struct _test_SetStruct_call {
    int32_t var_int32;
    uint32_t var_uint32;
    int64_t var_int64;
    uint64_t var_uint64;
    pb_size_t var_uint32_array_count;
    uint32_t var_uint32_array[8];
    bool var_bool;
    char var_string[16];
    test_SetStruct_call_var_bytes_t var_bytes;
} test_SetStruct_call;

typedef struct _test_SetStruct_reply {
    char dummy_field;
} test_SetStruct_reply;

typedef struct _test_TestCallset {
    pb_size_t which_msg;
    union {
        test_Add_call add_call;
        test_Add_reply add_reply;
        test_SetStruct_call setstruct_call;
        test_SetStruct_reply setstruct_reply;
        test_HandlerError_call handlererror_call;
        test_HandlerError_reply handlererror_reply;
    } msg;
} test_TestCallset;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define test_Add_call_init_default               {0, 0}
#define test_Add_reply_init_default              {0}
#define test_HandlerError_call_init_default      {0}
#define test_HandlerError_reply_init_default     {0}
#define test_SetStruct_call_init_default         {0, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, "", {0, {0}}}
#define test_SetStruct_reply_init_default        {0}
#define test_TestCallset_init_default            {0, {test_Add_call_init_default}}
#define test_Add_call_init_zero                  {0, 0}
#define test_Add_reply_init_zero                 {0}
#define test_HandlerError_call_init_zero         {0}
#define test_HandlerError_reply_init_zero        {0}
#define test_SetStruct_call_init_zero            {0, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, "", {0, {0}}}
#define test_SetStruct_reply_init_zero           {0}
#define test_TestCallset_init_zero               {0, {test_Add_call_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define test_Add_call_a_tag                      1
#define test_Add_call_b_tag                      2
#define test_Add_reply_sum_tag                   1
#define test_SetStruct_call_var_int32_tag        1
#define test_SetStruct_call_var_uint32_tag       2
#define test_SetStruct_call_var_int64_tag        3
#define test_SetStruct_call_var_uint64_tag       4
#define test_SetStruct_call_var_uint32_array_tag 5
#define test_SetStruct_call_var_bool_tag         6
#define test_SetStruct_call_var_string_tag       7
#define test_SetStruct_call_var_bytes_tag        8
#define test_TestCallset_add_call_tag            1
#define test_TestCallset_add_reply_tag           2
#define test_TestCallset_setstruct_call_tag      3
#define test_TestCallset_setstruct_reply_tag     4
#define test_TestCallset_handlererror_call_tag   5
#define test_TestCallset_handlererror_reply_tag  6

/* Struct field encoding specification for nanopb */
#define test_Add_call_FIELDLIST(X, a_) \
X(a_, STATIC,   SINGULAR, INT32,    a,                 1) \
X(a_, STATIC,   SINGULAR, INT32,    b,                 2)
#define test_Add_call_CALLBACK NULL
#define test_Add_call_DEFAULT NULL

#define test_Add_reply_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    sum,               1)
#define test_Add_reply_CALLBACK NULL
#define test_Add_reply_DEFAULT NULL

#define test_HandlerError_call_FIELDLIST(X, a) \

#define test_HandlerError_call_CALLBACK NULL
#define test_HandlerError_call_DEFAULT NULL

#define test_HandlerError_reply_FIELDLIST(X, a) \

#define test_HandlerError_reply_CALLBACK NULL
#define test_HandlerError_reply_DEFAULT NULL

#define test_SetStruct_call_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    var_int32,         1) \
X(a, STATIC,   SINGULAR, UINT32,   var_uint32,        2) \
X(a, STATIC,   SINGULAR, INT64,    var_int64,         3) \
X(a, STATIC,   SINGULAR, UINT64,   var_uint64,        4) \
X(a, STATIC,   REPEATED, UINT32,   var_uint32_array,   5) \
X(a, STATIC,   SINGULAR, BOOL,     var_bool,          6) \
X(a, STATIC,   SINGULAR, STRING,   var_string,        7) \
X(a, STATIC,   SINGULAR, BYTES,    var_bytes,         8)
#define test_SetStruct_call_CALLBACK NULL
#define test_SetStruct_call_DEFAULT NULL

#define test_SetStruct_reply_FIELDLIST(X, a) \

#define test_SetStruct_reply_CALLBACK NULL
#define test_SetStruct_reply_DEFAULT NULL

#define test_TestCallset_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MESSAGE,  (msg,add_call,msg.add_call),   1) \
X(a, STATIC,   ONEOF,    MESSAGE,  (msg,add_reply,msg.add_reply),   2) \
X(a, STATIC,   ONEOF,    MESSAGE,  (msg,setstruct_call,msg.setstruct_call),   3) \
X(a, STATIC,   ONEOF,    MESSAGE,  (msg,setstruct_reply,msg.setstruct_reply),   4) \
X(a, STATIC,   ONEOF,    MESSAGE,  (msg,handlererror_call,msg.handlererror_call),   5) \
X(a, STATIC,   ONEOF,    MESSAGE,  (msg,handlererror_reply,msg.handlererror_reply),   6)
#define test_TestCallset_CALLBACK NULL
#define test_TestCallset_DEFAULT NULL
#define test_TestCallset_msg_add_call_MSGTYPE test_Add_call
#define test_TestCallset_msg_add_reply_MSGTYPE test_Add_reply
#define test_TestCallset_msg_setstruct_call_MSGTYPE test_SetStruct_call
#define test_TestCallset_msg_setstruct_reply_MSGTYPE test_SetStruct_reply
#define test_TestCallset_msg_handlererror_call_MSGTYPE test_HandlerError_call
#define test_TestCallset_msg_handlererror_reply_MSGTYPE test_HandlerError_reply

extern const pb_msgdesc_t test_Add_call_msg;
extern const pb_msgdesc_t test_Add_reply_msg;
extern const pb_msgdesc_t test_HandlerError_call_msg;
extern const pb_msgdesc_t test_HandlerError_reply_msg;
extern const pb_msgdesc_t test_SetStruct_call_msg;
extern const pb_msgdesc_t test_SetStruct_reply_msg;
extern const pb_msgdesc_t test_TestCallset_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define test_Add_call_fields &test_Add_call_msg
#define test_Add_reply_fields &test_Add_reply_msg
#define test_HandlerError_call_fields &test_HandlerError_call_msg
#define test_HandlerError_reply_fields &test_HandlerError_reply_msg
#define test_SetStruct_call_fields &test_SetStruct_call_msg
#define test_SetStruct_reply_fields &test_SetStruct_reply_msg
#define test_TestCallset_fields &test_TestCallset_msg

/* Maximum encoded size of messages (where known) */
#define test_Add_call_size                       22
#define test_Add_reply_size                      11
#define test_HandlerError_call_size              0
#define test_HandlerError_reply_size             0
#define test_SetStruct_call_size                 124
#define test_SetStruct_reply_size                0
#define test_TestCallset_size                    126

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif