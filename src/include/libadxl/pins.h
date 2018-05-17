#ifndef LIBADXL_PINS_H
#define LIBADXL_PINS_H

// Assignments from libwispbase (for WISP5, MSP430FR5969)
// TODO: change these to be configurable from Makefile

// P2.4 - ACCEL_SCLK - OUTPUT
#define 	PIN_ACCEL_SCLK			(BIT4)
#define 	PDIR_ACCEL_SCLK			(P2DIR)
#define		PACCEL_SCLKSEL0			(P2SEL0)
#define		PACCEL_SCLKSEL1			(P2SEL1)

// P2.5 - ACCEL_MOSI - OUTPUT
#define 	PIN_ACCEL_MOSI			(BIT5)
#define 	PDIR_ACCEL_MOSI			(P2DIR)
#define		PACCEL_MOSISEL0			(P2SEL0)
#define		PACCEL_MOSISEL1			(P2SEL1)


// P2.6 - ACCEL_MISO - INPUT
#define 	PIN_ACCEL_MISO			(BIT6)
#define 	PDIR_ACCEL_MISO			(P2DIR)
#define		PACCEL_MISOSEL0			(P2SEL0)
#define		PACCEL_MISOSEL1			(P2SEL1)

// P4.2 - ACCEL_EN - OUTPUT
#define PIN_ACCEL_EN		BIT2
#define POUT_ACCEL_EN		P4OUT
#define PDIR_ACCEL_EN		P4DIR

// P4.3 - ACCEL_CS - OUTPUT
#define PIN_ACCEL_CS		BIT3
#define POUT_ACCEL_CS		P4OUT
#define PDIR_ACCEL_CS		P4DIR

#endif // LIBADXL_PINS_H
