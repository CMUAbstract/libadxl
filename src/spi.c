#include <msp430.h>

#include <stdint.h>
#include <stdbool.h>

#include <libadxl/spi.h>

/**
 * Description of state of the SPI module.
 */
static struct {
#ifdef SPI_SYNC
    bool bPortInUse;
#endif // SPI_SYNC
    bool bNewDataReceived;
    unsigned int uiCurRx;
    unsigned int uiCurTx;
    unsigned int uiBytesToSend;
    uint8_t *pcRxBuffer;
    uint8_t *pcTxBuffer;
} spiSM;


/**
 *
 * @return success or failure
 *
 * @todo Implement this function
 */
bool SPI_initialize() {


    // Hardware peripheral initialization
    BITSET(UCA1CTLW0, UCSWRST);

    UCA1CTLW0 = UCMST | UCSYNC | UCCKPH | UCMSB | UCSSEL__SMCLK | UCSWRST;
    UCA1BRW = 20; // 8MHz / 400KHz
    UCA1IFG = 0;
    BITCLR(UCA1CTLW0, UCSWRST);

    // State variable initialization
#ifdef SPI_SYNC
    spiSM.bPortInUse = false;
#endif // SPI_SYNC
    spiSM.bNewDataReceived = false;
    spiSM.uiCurRx = 0;
    spiSM.uiCurTx = 0;
    spiSM.uiBytesToSend = 0;


    return SUCCESS;
}

#ifdef SPI_SYNC
/**
 *
 * @return Success - you were able to get the port. Fail - you don't have the port, so don't use it.
 */
bool SPI_acquirePort() {

    if(spiSM.bPortInUse) {
        return FAIL;
    } else {
        spiSM.bPortInUse=TRUE;
        return SUCCESS;
    }

}

/**
 *
 * @return success or fail
 * @todo Make this more robust (don't allow release of port if we don't have it)
 */
bool SPI_releasePort() {
    if(spiSM.bPortInUse) {
        spiSM.bPortInUse = false;
        return SUCCESS;
    }
    return FAIL;
}
#endif // SPI_SYNC

void SPI_waitForRx() {
    while(!(UCA1IFG & UCRXIFG));
}

/**
 * Engage in a synchronous serial transaction of the specified length.
 * This function blocks until transaction is complete.
 *
 * @param txBuf
 * @param size
 * @return success or fail
 */
bool SPI_transaction(uint8_t* rxBuf, uint8_t* txBuf, uint16_t size) {

#ifdef SPI_SYNC
    if(!spiSM.bPortInUse)
        return FAIL; // If the port is not acquired, fail!
#endif // SPI_SYNC

    if(size==0)
        return FAIL; // If we aren't sending anything, fail!

    spiSM.bNewDataReceived = false;
    spiSM.uiCurRx = 0;
    spiSM.uiCurTx = 0;
    spiSM.uiBytesToSend = size;

    spiSM.pcRxBuffer = rxBuf;
    spiSM.pcTxBuffer = txBuf;

    //BITSET(UCA1IE, UCTXIE | UCRXIE);

    for (; spiSM.uiBytesToSend > 0; --spiSM.uiBytesToSend) {
        // Reset receive flag
        spiSM.bNewDataReceived = false;

        // Start transmission
        UCA1TXBUF = spiSM.pcTxBuffer[spiSM.uiCurTx];

        SPI_waitForRx();

        spiSM.pcRxBuffer[spiSM.uiCurRx] = UCA1RXBUF;
        UCA1IFG &= ~UCRXIFG;
        UCA1IFG &= ~UCTXIFG;
        // Move to next TX and RX index
        spiSM.uiCurTx++;
        spiSM.uiCurRx++;
    }

    return SUCCESS;
}

