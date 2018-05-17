#ifndef LIBADXL_ADXL362_H
#define LIBADXL_ADXL362_H

#include <stdint.h>

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} threeAxis_t;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} threeAxis_t_8;

BOOL ACCEL_initialize();
#ifdef ACCEL_16BIT_TYPE
BOOL ACCEL_singleSample(threeAxis_t* result);
BOOL ACCEL_readStat(threeAxis_t* result);
BOOL ACCEL_readID(threeAxis_t* result);
#else
BOOL ACCEL_singleSample(threeAxis_t_8* result);
BOOL ACCEL_readStat(threeAxis_t_8* result);
BOOL ACCEL_readID(threeAxis_t_8* result);
#endif
BOOL ACCEL_reset();
BOOL ACCEL_range();

#endif // LIBADXL_ADXL362_H