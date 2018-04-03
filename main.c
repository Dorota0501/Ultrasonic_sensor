#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "UART.h"

#define dioda PB2

#define echo PD2
#define trigger PB1

#define rxd PD0
#define txd PD1

volatile uint16_t timer = 0;
volatile uint8_t time_out = 0;
volatile uint8_t current_state = 0;

volatile uint8_t jestPomiar = 0;

void pin_init() {
	DDRB |= (1 << trigger) | (1 << dioda);
	PORTB &= (0 << dioda);	//dioda nie właczonaa

	// UART
	DDRD |= (1 << txd) | (0 << rxd) | (0 << echo);	//rxd- 0, txd- 1, int0-0
	PORTD |= (1 << rxd) | (1 << echo); 				//pull up

}

void interrupt_init() {
	//SREG = (1 << I);	//global interrupt enabel NOT FOUND
	MCUCR = (1 << ISC00) | (0 << ISC01);

	GICR |= (1 << INT0);
	//GIFR |= (1 << INTF0);	//sztucznie wywołane przerwanie
}
void timer_config() {
	//preskaler 1024
	TCCR1B |= (1 << CS10) | (1 << CS12);
	timer = TCNT1H | TCNT1L;
}

int main() {

	pin_init();
	USART_Init(MY_UBRR);
	interrupt_init();
	_delay_ms(100);
	timer_config();
	sei();

	//jezeli ktoras z flag wystapi wyswitlam komunikat,
	//timeout albo wynik pomiaru i zaczynam nowy pomiar

	while (1) {
		PORTB |= (1 << PB2);		//dioda wyłączona sterowanie na odwrót
		if (time_out) {
			USART_Transmit('T');
			USART_Transmit('\n');

			//wyswietlam wartowsc licznika
			char tab[10];
			ltoa(timer, tab, 10);

			for (int i = 0; i < 10; i++) {
				USART_Transmit(tab[i]);	//wysweitl wartosc licznika
			}
			USART_Transmit('\n');
			// -----------------------------URUCAMIAN CZUJNIK-----------------
			PORTB = (PORTB & 0b0100);
			_delay_us(5);

			PORTB = (PORTB & 0b0110); //trigger czujnika musi być ustawiony na 10us na 5V
			_delay_us(10);

			PORTB = (PORTB & 0b0100);
			time_out = 0;
		}
		if (jestPomiar) {
			USART_Transmit('P');
			USART_Transmit('\n');

			//wyswietlam wartowsc licznika
			char tab[10];
			itoa(timer, tab, 10);

			for (int i = 0; i < 10; i++) {
				USART_Transmit(tab[i]);	//wysweitl wartosc licznika
			}
			USART_Transmit('\n');
			// -----------------------------URUCAMIAN CZUJNIK-----------------
			PORTB = (PORTB & 0b0100);
			_delay_us(5);

			PORTB = (PORTB & 0b0110); //trigger czujnika musi być ustawiony na 10us na 5V
			_delay_us(10);

			PORTB = (PORTB & 0b0100);
			jestPomiar = 0;
		}
	}
	return 0;
}

ISR(INT0_vect) {	//zmieniony został stan naa pinie echo
	current_state = PIND & 0b00000100;
	if (current_state == 0x04) //nastąpiło zbocze narastające
			{
		TCNT1 = 0;	//zeruj licznik
		PORTB &= (0 << dioda);
	}

//wchodzi do przerwania ale nie wchodzi do drugiego ifa,
//odczytywanie zboczy nie działa

	else {
		timer = TCNT1;	//odczytaj wartosc z licznika
		jestPomiar = 1;
	}

}

ISR(TIMER0_OVF_vect) {
	time_out = 1;
}

