#ifndef LIBADXL_SPI_H
#define LIBADXL_SPI_H

#include <stdint.h>
#include <stdbool.h>

#define FAIL            (0)
#define SUCCESS         (1)

#define BITSET(port,pin)    port |= (pin)
#define BITCLR(port,pin)    port &= ~(pin)
#define BITTOG(port,pin)    port ^= (pin)

bool SPI_initialize();
bool SPI_acquirePort();
bool SPI_releasePort();
bool SPI_transaction(uint8_t* rxBuf, uint8_t* txBuf, uint16_t size);

#ifdef __MSPGCC__
#define USCI_UCRXIFG UCRXIFG
#define USCI_UCTXIFG UCTXIFG
#endif /* __MSPGCC__ */

#endif // LIBADXL_SPI_H
