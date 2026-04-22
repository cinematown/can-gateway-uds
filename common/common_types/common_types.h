/**
 * @file    common_types.h
 * @brief   프로젝트 전반 공용 타입/매크로
 */

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* ARRAY_SIZE — 표준 아님. 공용 매크로로 제공. */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))
#endif

/* 비트 조작 */
#define BIT_SET(val, bit)    ((val) |=  (1U << (bit)))
#define BIT_CLR(val, bit)    ((val) &= ~(1U << (bit)))
#define BIT_GET(val, bit)    (((val) >> (bit)) & 1U)

/* Endian 변환 (Big-Endian 인코딩용) */
#define U16_TO_BE(u, buf)    do { \
    (buf)[0] = (uint8_t)((u) >> 8); \
    (buf)[1] = (uint8_t)((u) & 0xFF); \
} while (0)

#define BE_TO_U16(buf)       (((uint16_t)(buf)[0] << 8) | (uint16_t)(buf)[1])

/* Little-Endian (VW Passat 0x280 등) */
#define U16_TO_LE(u, buf)    do { \
    (buf)[0] = (uint8_t)((u) & 0xFF); \
    (buf)[1] = (uint8_t)((u) >> 8); \
} while (0)

#define LE_TO_U16(buf)       ((uint16_t)(buf)[0] | ((uint16_t)(buf)[1] << 8))

/* 범위 제한 */
#define CLAMP(v, lo, hi)     ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))

#endif /* COMMON_TYPES_H */
