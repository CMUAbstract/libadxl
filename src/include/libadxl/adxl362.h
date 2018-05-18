#ifndef LIBADXL_ADXL362_H
#define LIBADXL_ADXL362_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t padding; // workaround for Clang compiler crash
} threeAxis_t;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t padding; // workaround for Clang compiler crash
} threeAxis_t_8;

void ACCEL_init();
bool ACCEL_initialize();
#ifdef ACCEL_16BIT_TYPE
bool ACCEL_singleSample(threeAxis_t* result);
bool ACCEL_readStat(threeAxis_t* result);
bool ACCEL_readID(threeAxis_t* result);
#else
bool ACCEL_singleSample(threeAxis_t_8* result);
bool ACCEL_readStat(threeAxis_t_8* result);
bool ACCEL_readID(threeAxis_t_8* result);
#endif
bool ACCEL_reset();
bool ACCEL_range();

#endif // LIBADXL_ADXL362_H
