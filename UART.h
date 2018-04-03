/*
 * UART.h
 *
 *  Created on: 7 mar 2018
 *      Author: dorota
 */

#ifndef UART_H_
#define UART_H_

#define MY_UBRR 207
// baud = 2400
void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);
unsigned char USART_Receive(void) ;
#endif /* UART_H_ */
