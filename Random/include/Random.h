/*******************************************************************************
 *  @file: Random.h
 *   
 *  @brief: Wrapper for esp_random utilities.
*******************************************************************************/
#ifndef RANDOM_H
#define RANDOM_H

#include "esp_random.h"

/** @brief Get a random number in the range 0 to 255. */
#define RANDOM8()           ((uint8_t)(((uint64_t)esp_random() * 255) >> 32))

/** @brief Get a random number in the range 0 to 2**16-1. */
#define RANDOM16()          ((uint16_t)(((uint64_t)esp_random() * 65535) >> 32))

/** @brief Get a random number in the range 0 to 2**32-1. */
#define RANDOM32()          esp_random()

/** @brief Get a random number in the range 0 to n-1 */
#define RANDOM(type, n)     ((type)(((uint64_t)esp_random() * ((n)-1)) >> 32))

/** @brief Get a random number between left and right. */
#define RANDOM8_RANGE(left, right)                                    \
({                                                                    \
    uint8_t delta = right - left;                                     \
    uint8_t ret = RANDOM(uint8_t, delta) + left;                      \
    ret;                                                              \
})

/** @brief Get a random 0 or 1. */
#define RANDOM_BIN()        (esp_random() & 0x80000000) ? 1 : 0

/** @brief Fill buffer with random bytes. */
#define RANDOM_FILL(buf, size)   esp_fill_random((void *)(buf), (size))

#endif
