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
#define RANDOM(n, type)     ((type)(((uint64_t)esp_random() * ((n)-1)) >> 32))

/** @brief Fill buffer with random bytes. */
#define RANDOM_FILL(buf, size)   esp_fill_random((void *)(buf), (size))

#endif
