/*******************************************************************************
 *  @file: PbGeneric.c
 *  
 *  @brief: General functions for Protobuf handling.
*******************************************************************************/
#include "esp_log.h"
#include "PbGeneric.h"

static const char *TAG = "PbGeneric";

#define LOGPRINT_ERROR(fmt, ...) \
    ESP_LOGE(TAG, "(l:%u) " fmt, __LINE__, ##__VA_ARGS__)

#define LOGPRINT_INFO(fmt, ...) \
    ESP_LOGI(TAG, "(l:%u) " fmt, __LINE__, ##__VA_ARGS__)

/******************************************************************************
    [docimport Pb_pack]
*//**
    @brief Packs a message struct to a buffer of bytes.
    @param[in] buf  Pointer to destination of packed protobuf message buffer.
    @param[in] buflen  Length of the buffer.
    @param[in] src  Pointer to the source struct to pack.
    @param[in] fields  Pointer to the protobuf message fields object.
******************************************************************************/
uint32_t
Pb_pack(uint8_t *buf, uint32_t buflen, void *src, void *fields)
{
    bool status;
    pb_ostream_t stream = pb_ostream_from_buffer(buf, buflen);
    status = pb_encode(&stream, (pb_msgdesc_t *)fields, src);
    if (!status)
    {
        LOGPRINT_ERROR("pb_encode failure: %s\r\n", PB_GET_ERROR(&stream));
        return 0;
    }
    return stream.bytes_written;
}

/******************************************************************************
    [docimport Pb_unpack]
*//**
    @brief Unpacks a protobuf blob.
    @param[in] buf  Pointer to packed protobuf message buffer.
    @param[in] len  Length of the message.
    @param[in] target  Pointer to the target struct to unpack into.
    @param[in] fields  Pointer to the protobuf message fields object.
******************************************************************************/
bool
Pb_unpack(uint8_t *buf, uint32_t len, void *target, void *fields)
{
    bool status;
    pb_istream_t stream = pb_istream_from_buffer(buf, len);
    
    status = pb_decode(&stream, (pb_msgdesc_t *)fields, target);
    if (!status)
    {
        LOGPRINT_ERROR("pb_decode failure: %s\r\n", PB_GET_ERROR(&stream));
    }
    return status;
}
