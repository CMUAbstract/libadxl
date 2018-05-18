#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include <libio/console.h>
#include <libmspbuiltins/builtins.h>

#include <libadxl/spi.h>
#include <libadxl/pins.h>
#include <libadxl/adxl362.h>

//
// Commands used for ADXL362 read/write access. Commands are sent prior to data/adx.
//
#define ADXL_CMD_WRITE_REG      0x0A
#define ADXL_CMD_READ_REG       0x0B
#define ADXL_CMD_READ_FIFO      0x0D

//
// Registers in the ADXL362. Created from the register map given in Table 11 of the datasheet.
//
#define ADXL_REG_DEVID_AD       0x00
#define ADXL_REG_DEVID_MST      0x01
#define ADXL_REG_PARTID         0x02
#define ADXL_REG_REVID          0x03
#define ADXL_REG_XDATA          0x08
#define ADXL_REG_YDATA          0x09
#define ADXL_REG_ZDATA          0x0A
#define ADXL_REG_STATUS         0x0B
#define ADXL_REG_FIFO_ENTRIES_L 0x0C
#define ADXL_REG_FIFO_ENTRIES_H 0x0D
#define ADXL_REG_XDATA_L        0x0E
#define ADXL_REG_XDATA_H        0x0F
#define ADXL_REG_YDATA_L        0x10
#define ADXL_REG_YDATA_H        0x11
#define ADXL_REG_ZDATA_L        0x12
#define ADXL_REG_ZDATA_H        0x13
#define ADXL_REG_TEMP_L         0x14
#define ADXL_REG_TEMP_H         0x15
#define ADXL_REG_Reserved0      0x16
#define ADXL_REG_Reserved1      0x17
#define ADXL_REG_SOFT_RESET     0x1F
#define ADXL_REG_THRESH_ACT_L   0x20
#define ADXL_REG_THRESH_ACT_H   0x21
#define ADXL_REG_TIME_ACT       0x22
#define ADXL_REG_THRESH_INACT_L 0x23
#define ADXL_REG_THRESH_INACT_H 0x24
#define ADXL_REG_TIME_INACT_L   0x25
#define ADXL_REG_TIME_INACT_H   0x26
#define ADXL_REG_ACT_INACT_CTL  0x27
#define ADXL_REG_FIFO_CONTROL   0x28
#define ADXL_REG_FIFO_SAMPLES   0x29
#define ADXL_REG_INTMAP1        0x2A
#define ADXL_REG_INTMAP2        0x2B
#define ADXL_REG_FILTER_CTL     0x2C
#define ADXL_REG_POWER_CTL      0x2D
#define ADXL_REG_SELF_TEST      0x2E

///////////////////////////////////////////////////////////////////////////////

// TODO Translate these to a better format once SPI driver is fleshed out. Make them CONST.
uint8_t const ADXL_READ_PARTID[] = {ADXL_CMD_READ_REG,ADXL_REG_PARTID,0x00};
uint8_t const ADXL_READ_DEVID[] = {ADXL_CMD_READ_REG,ADXL_REG_DEVID_AD,0x00};
uint8_t const ADXL_REAsxD_STATUS[] = {ADXL_CMD_READ_REG,ADXL_REG_STATUS,0x00};
uint8_t const ADXL_READ_XYZ_8BIT[] = {ADXL_CMD_READ_REG,ADXL_REG_XDATA,0x00,0x00,0x00};
uint8_t const ADXL_READ_XYZ_16BIT[] = {ADXL_CMD_READ_REG,ADXL_REG_XDATA_L,0x00,0x00,0x00,0x00,0x00,0x00};
uint8_t const ADXL_CONFIG_MEAS[] = {ADXL_CMD_WRITE_REG,ADXL_REG_POWER_CTL,0x02}; // Put the ADXL into measurement mode
uint8_t const ADXL_CONFIG_STBY[] = {ADXL_CMD_WRITE_REG,ADXL_REG_POWER_CTL,0x00}; // Put the ADXL into standby mode
uint8_t const ADXL_CONFIG_RESET[] = {ADXL_CMD_WRITE_REG,ADXL_REG_SOFT_RESET,0x52};
uint8_t const ADXL_CONFIG_FILTER[] = {ADXL_CMD_WRITE_REG,ADXL_REG_FILTER_CTL,0x17};
///////////////////////////////////////////////////////////////////////////////

#define MAX2(a,b) ((a) < (b) ? (b) : (a))
#define MAX4(a,b,c,d) MAX2(MAX2(a,b),MAX2(c,d))
#define MAX8(a,b,c,d,e,f,g,h) MAX2(MAX4(a,b,c,d),MAX4(e,f,g,h))
#define MAX9(a,b,c,d,e,f,g,h,i) MAX2(MAX8(a,b,c,d,e,f,g,h),i)

#define MAX_SPI_TX_SIZE MAX9(\
    sizeof(ADXL_READ_PARTID),\
    sizeof(ADXL_READ_DEVID),\
    sizeof(ADXL_REAsxD_STATUS),\
    sizeof(ADXL_READ_XYZ_8BIT),\
    sizeof(ADXL_READ_XYZ_16BIT),\
    sizeof(ADXL_CONFIG_MEAS),\
    sizeof(ADXL_CONFIG_STBY),\
    sizeof(ADXL_CONFIG_RESET),\
    sizeof(ADXL_CONFIG_FILTER))

#define SPI_GP_RXBUF_SIZE 20
static uint8_t gpRxBuf[SPI_GP_RXBUF_SIZE];

void ACCEL_init()
{
    threeAxis_t_8 accelID = {0};

    LOG("init: initializing accel\r\n");

    /*
    SPI_initialize();
    ACCEL_initialize();
    */
    // ACCEL_SetReg(0x2D,0x02);

    /* TODO: move the below stuff to accel.c */
    BITSET(P4OUT, PIN_ACCEL_EN);

    BITSET(P2SEL1, PIN_ACCEL_SCLK | PIN_ACCEL_MISO | PIN_ACCEL_MOSI);
    BITCLR(P2SEL0, PIN_ACCEL_SCLK | PIN_ACCEL_MISO | PIN_ACCEL_MOSI);
    __delay_cycles(1000);
    SPI_initialize();
    __delay_cycles(1000);
    ACCEL_range();
    __delay_cycles(1000);
    ACCEL_initialize();
    __delay_cycles(1000);
    ACCEL_readID(&accelID);

    LOG("init: accel hw id: 0x%x\r\n", accelID.x);
}


bool ACCEL_initialize_withoutWait() {

    // TODO Figure out optimal ADXL configuration for single measurement

#ifdef SPI_SYNC
    while(!SPI_acquirePort());
#endif // SPI_SYNC

    //BITSET(POUT_ACCEL_EN, PIN_ACCEL_EN);
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);
    BITCLR(POUT_ACCEL_CS, PIN_ACCEL_CS);

    SPI_transaction(gpRxBuf, (uint8_t*)ADXL_CONFIG_MEAS, sizeof(ADXL_CONFIG_MEAS));
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

#ifdef SPI_SYNC
    SPI_releasePort();
#endif // SPI_SYNC

    //Timer_LooseDelay(LP_LSDLY_200MS);// To let ADXL start measuring? How much time is actually required here, if any?

    // TODO Use a pin interrupt to let us know when first measurement is ready
    return SUCCESS;
}

/**
 * Put the ADXL362 into a lower power standby state without gating power
 *
 * @todo Implement this function
 */
void ACCEL_standby() {


}

/**
 * Grab one sample from the ADXL362 accelerometer
 */
bool ACCEL_singleSample(threeAxis_t_8* result) {

#ifdef SPI_SYNC
    while(!SPI_acquirePort());
#endif // SPI_SYNC

    BITCLR(POUT_ACCEL_CS, PIN_ACCEL_CS);
    SPI_transaction(gpRxBuf, (uint8_t*)ADXL_READ_XYZ_8BIT, sizeof(ADXL_READ_XYZ_8BIT));
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

#ifdef SPI_SYNC
    SPI_releasePort();
#endif // SPI_SYNC

    result->x = gpRxBuf[2];
    result->y = gpRxBuf[3];
    result->z = gpRxBuf[4];

    return SUCCESS;
}

bool ACCEL_readStat(threeAxis_t_8* result) {

#ifdef SPI_SYNC
   while(!SPI_acquirePort());
#endif // SPI_SYNC

    BITCLR(POUT_ACCEL_CS, PIN_ACCEL_CS);
    SPI_transaction(gpRxBuf, (uint8_t*)ADXL_REAsxD_STATUS, sizeof(ADXL_REAsxD_STATUS));
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

#ifdef SPI_SYNC
    SPI_releasePort();
#endif // SPI_SYNC

    result->x = gpRxBuf[2];

    return SUCCESS;
}

bool ACCEL_readID(threeAxis_t_8* result) {

#ifdef SPI_SYNC
    while(!SPI_acquirePort());
#endif // SPI_SYNC

    BITCLR(POUT_ACCEL_CS, PIN_ACCEL_CS);
    SPI_transaction(gpRxBuf, (uint8_t*)ADXL_READ_DEVID, sizeof(ADXL_READ_DEVID));
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

#ifdef SPI_SYNC
    SPI_releasePort();
#endif // SPI_SYNC

    result->x = gpRxBuf[2];

    return SUCCESS;
}

/**
 * Turn on and start up the ADXL362 accelerometer. This leaves the ADXL running.
 */

bool ACCEL_reset() {

    // TODO Figure out optimal ADXL configuration for single measurement

#ifdef SPI_SYNC
    while(!SPI_acquirePort());
#endif // SPI_SYNC

    //BITSET(POUT_ACCEL_EN, PIN_ACCEL_EN);
    //BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

    //TODO find the proper length of delay

    BITCLR(POUT_ACCEL_CS, PIN_ACCEL_CS);
    SPI_transaction(gpRxBuf, (uint8_t*)ADXL_CONFIG_RESET, sizeof(ADXL_CONFIG_RESET));
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

#ifdef SPI_SYNC
    SPI_releasePort();
#endif // SPI_SYNC

    //__delay_cycles(5000);

    //Timer_LooseDelay(LP_LSDLY_200MS);// To let ADXL start measuring? How much time is actually required here, if any?

    // TODO Use a pin interrupt to let us know when first measurement is ready
    return SUCCESS;
}

bool ACCEL_range() {

    // TODO Figure out optimal ADXL configuration for single measurement

#ifdef SPI_SYNC
    while(!SPI_acquirePort());
#endif // SPI_SYNC

    //BITSET(POUT_ACCEL_EN, PIN_ACCEL_EN);
    //BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

    //TODO find the proper length of delay

    BITCLR(POUT_ACCEL_CS, PIN_ACCEL_CS);
    SPI_transaction(gpRxBuf, (uint8_t*)ADXL_CONFIG_FILTER, sizeof(ADXL_CONFIG_FILTER));
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

#ifdef SPI_SYNC
    SPI_releasePort();
#endif // SPI_SYNC

    //__delay_cycles(5000);

    //Timer_LooseDelay(LP_LSDLY_200MS);// To let ADXL start measuring? How much time is actually required here, if any?

    // TODO Use a pin interrupt to let us know when first measurement is ready
    return SUCCESS;
}


bool ACCEL_initialize() {

    // TODO Figure out optimal ADXL configuration for single measurement

#ifdef SPI_SYNC
    while(!SPI_acquirePort());
#endif // SPI_SYNC

    //BITSET(POUT_ACCEL_EN, PIN_ACCEL_EN);
    //BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

    //TODO find the proper length of delay

    BITCLR(POUT_ACCEL_CS, PIN_ACCEL_CS);
    SPI_transaction(gpRxBuf, (uint8_t*)ADXL_CONFIG_MEAS, sizeof(ADXL_CONFIG_MEAS));
    BITSET(POUT_ACCEL_CS, PIN_ACCEL_CS);

#ifdef SPI_SYNC
    SPI_releasePort();
#endif // SPI_SYNC

    //__delay_cycles(5000);

    //Timer_LooseDelay(LP_LSDLY_200MS);// To let ADXL start measuring? How much time is actually required here, if any?

    // TODO Use a pin interrupt to let us know when first measurement is ready
    return SUCCESS;
}
