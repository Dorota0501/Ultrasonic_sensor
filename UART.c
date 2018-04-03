/*
 * UART.c
 *
 *  Created on: 7 mar 2018
 *      Author: dorota
 */

#include <avr/io.h>

void USART_Init(unsigned int ubrr) {

	UBRRH = (unsigned char) (ubrr >> 8);
	UBRRL = (unsigned char) ubrr;
	UCSRB = (1 << RXEN) | (1 << TXEN);
	UCSRC = (1 << URSEL) | (1 << USBS) | (3 << UCSZ0);
}

void USART_Transmit(unsigned char data) {
	while (!( UCSRA & (1 << UDRE)));
	UDR = data;
}

unsigned char USART_Receive(void) {
	while (!(UCSRA & (1 << RXC)));
	return UDR;
}

