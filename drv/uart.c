/* name:    serial driver
 * desc:    simple serial driver
 * arch:    AVR CPU, optimized for ATmega8
 * author:  (c)2012 Pavel Revak <pavel.revak@gmail.com>
 * licence: GPL
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "lib/cpudefs.h"
#include "uart.h"

#define ASCII_MODE

/* alloved values 2, 4, 8, 16, 32, 64, 128, 256 */
#define BUFFER_SIZE	(64)

#define BUFFER_MASK	(BUFFER_SIZE - 1)
#if (BUFFER_SIZE > 256)
#error BUFFER_SIZE exceed max size (256Bytes)
#endif
#if (BUFFER_SIZE & BUFFER_MASK)
#error BUFFER_SIZE is not a power of 2
#endif

static uint8_t rx_buffer[BUFFER_SIZE];
static uint8_t rx_tail;
static volatile uint8_t rx_head;

void uartPutChar(char c) {
#ifdef ASCII_MODE
	if (c == '\n') uartPutChar('\r');
#endif
	waitUDRE();
	setUDR(c);
}

ISR(USART_RX_vect) {
	uint8_t ch = getUDR();
	if (((rx_head + 1) & BUFFER_MASK) == rx_tail) return;
	rx_buffer[rx_head++] = ch;
	rx_head &= BUFFER_MASK;
}

char uartIsChar() {
	return (rx_head != rx_tail);
}

char uartGetChar() {
	if (rx_head != rx_tail) {
		uint8_t ch = rx_buffer[rx_tail++]; rx_tail &= BUFFER_MASK;
		return ch;
	}
	return 0;
}

void uartOpen(uint32_t baud) {
	uint16_t br = ((F_CPU + baud * 4UL) / (baud * 8UL) - 1);
	rx_head = 0;
	rx_tail = 0;
	setUBRRH((uint8_t)(br >> 8));
	setUBRRL((uint8_t)(br & 0x00ff));
	setUCSRA(BV_U2X);
	setUCSRC(BV_UCSZ1 | BV_UCSZ0);
	setUCSRB(BV_TXEN | BV_RXEN | BV_RXCIE);
}
