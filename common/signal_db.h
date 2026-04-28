#ifndef SIGNAL_DB_H
#define SIGNAL_DB_H

#include <stdint.h>
#include <string.h>

#define SIGNAL_DB_CAN_ID_MOTOR_1      0x280u
#define SIGNAL_DB_CAN_ID_BREMSE_1     0x1A0u
#define SIGNAL_DB_CAN_ID_MOTOR_2      0x288u
#define SIGNAL_DB_CAN_ID_KOMBI_1      0x320u
#define SIGNAL_DB_CAN_ID_KOMBI_2      0x420u

#define SIGNAL_DB_MOTOR_1_RPM_START       16u
#define SIGNAL_DB_MOTOR_1_RPM_LENGTH      16u
#define SIGNAL_DB_BREMSE_1_SPEED_START    17u
#define SIGNAL_DB_BREMSE_1_SPEED_LENGTH   15u
#define SIGNAL_DB_MOTOR_2_COOLANT_START   8u
#define SIGNAL_DB_MOTOR_2_COOLANT_LENGTH  8u

static inline void SignalDb_ClearFrame(uint8_t data[8])
{
    memset(data, 0, 8);
}

static inline void SignalDb_SetLeSignal(uint8_t *data, uint8_t start_bit,
                                        uint8_t bit_len, uint32_t raw)
{
    for (uint8_t i = 0; i < bit_len; i++) {
        uint8_t bit_pos = (uint8_t)(start_bit + i);
        uint8_t byte_idx = (uint8_t)(bit_pos / 8u);
        uint8_t bit_idx = (uint8_t)(bit_pos % 8u);

        if ((raw & (1u << i)) != 0u) {
            data[byte_idx] |= (uint8_t)(1u << bit_idx);
        } else {
            data[byte_idx] &= (uint8_t)~(1u << bit_idx);
        }
    }
}

static inline uint32_t SignalDb_GetLeSignal(const uint8_t *data,
                                            uint8_t start_bit,
                                            uint8_t bit_len)
{
    uint32_t raw = 0;

    for (uint8_t i = 0; i < bit_len; i++) {
        uint8_t bit_pos = (uint8_t)(start_bit + i);
        uint8_t byte_idx = (uint8_t)(bit_pos / 8u);
        uint8_t bit_idx = (uint8_t)(bit_pos % 8u);

        if ((data[byte_idx] & (uint8_t)(1u << bit_idx)) != 0u) {
            raw |= (1u << i);
        }
    }

    return raw;
}

static inline void SignalDb_EncodeMotor1Rpm(uint8_t data[8], uint16_t rpm)
{
    SignalDb_ClearFrame(data);
    SignalDb_SetLeSignal(data, SIGNAL_DB_MOTOR_1_RPM_START,
                         SIGNAL_DB_MOTOR_1_RPM_LENGTH, (uint32_t)rpm * 4u);
}

static inline uint16_t SignalDb_DecodeMotor1Rpm(const uint8_t data[8])
{
    uint32_t raw = SignalDb_GetLeSignal(data, SIGNAL_DB_MOTOR_1_RPM_START,
                                        SIGNAL_DB_MOTOR_1_RPM_LENGTH);
    return (uint16_t)(raw / 4u);
}

static inline void SignalDb_EncodeBremse1Speed(uint8_t data[8], uint16_t kmh)
{
    SignalDb_ClearFrame(data);
    SignalDb_SetLeSignal(data, SIGNAL_DB_BREMSE_1_SPEED_START,
                         SIGNAL_DB_BREMSE_1_SPEED_LENGTH, (uint32_t)kmh * 100u);
}

static inline uint16_t SignalDb_DecodeBremse1Speed(const uint8_t data[8])
{
    uint32_t raw = SignalDb_GetLeSignal(data, SIGNAL_DB_BREMSE_1_SPEED_START,
                                        SIGNAL_DB_BREMSE_1_SPEED_LENGTH);
    return (uint16_t)(raw / 100u);
}

static inline void SignalDb_EncodeMotor2Coolant(uint8_t data[8], uint8_t celsius)
{
    uint32_t raw = (((uint32_t)celsius + 48u) * 4u) / 3u;

    SignalDb_ClearFrame(data);
    SignalDb_SetLeSignal(data, SIGNAL_DB_MOTOR_2_COOLANT_START,
                         SIGNAL_DB_MOTOR_2_COOLANT_LENGTH, raw);
}

static inline uint8_t SignalDb_DecodeMotor2Coolant(const uint8_t data[8])
{
    uint32_t raw = SignalDb_GetLeSignal(data, SIGNAL_DB_MOTOR_2_COOLANT_START,
                                        SIGNAL_DB_MOTOR_2_COOLANT_LENGTH);
    int32_t celsius = ((int32_t)raw * 3) / 4 - 48;

    if (celsius < 0) {
        return 0;
    }

    if (celsius > 255) {
        return 255;
    }

    return (uint8_t)celsius;
}

static inline void CAN_PutU16LE(uint8_t *buf, uint8_t index, uint16_t value)
{
    buf[index] = (uint8_t)(value & 0xFFu);
    buf[index + 1u] = (uint8_t)((value >> 8) & 0xFFu);
}

static inline uint16_t CAN_GetU16LE(const uint8_t *buf, uint8_t index)
{
    return (uint16_t)(((uint16_t)buf[index + 1u] << 8) | buf[index]);
}

#endif
