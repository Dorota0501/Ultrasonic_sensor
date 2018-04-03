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

volatile uint16_t timer = 0;		//zerowanie wartosci zczytywanej z licznika
volatile uint8_t time_out = 0;		//flaga zbyt długiego czasu na pomiar
volatile uint8_t current_state = 0;	//aktualna wartość portu D
volatile uint8_t jestPomiar = 0;	//nastąpiło zbocze narastające,
									//pomiar się zakończył


//-------------------------------------------------------------
void pin_init() {
	DDRB |= (1 << trigger) | (1 << dioda);
	PORTB &= (0 << dioda);							//wyłącz diode

	// UART
	DDRD |= (1 << txd) | (0 << rxd) | (0 << echo);	//rxd- 0, txd- 1, int0-0
	PORTD |= (1 << rxd) | (1 << echo); 				//pull up

}

void interrupt_init() {
	MCUCR = (1 << ISC00) | (0 << ISC01);
	GICR |= (1 << INT0);
}

void timer1_config() {

	TCCR1B |= (1 << CS10) | (1 << CS12);	//preskaler 1024
	TIMSK |= (1 << TOIE1);					//zezwolenie na przerwanie
											// ^ przepełnienia licznika
	timer = TCNT1H | TCNT1L;

}

void make_measure() {
	// -----------------------------URUCAMIAN CZUJNIK-----------------
	PORTB = (PORTB & 0b0100);
	_delay_us(5);

	PORTB = (PORTB & 0b0110); //trigger czujnika musi być ustawiony na 10us na 5V
	_delay_us(10);

	PORTB = (PORTB & 0b0100);
}

void show_timer_counter() {
	//wyswietlam wartowsc licznika
	char tab[5];
	ltoa(timer, tab, 10);

	for (int i = 0; i < 5; i++) {
		USART_Transmit(tab[i]);	//wysweitl wartosc licznika
	}
	USART_Transmit('\n');
}

int main() {

	pin_init();
	USART_Init(MY_UBRR);
	interrupt_init();
	_delay_ms(100);
	timer1_config();
	sei();

	//jezeli ktoras z flag wystapi wyswitlam komunikat,
	//timeout albo wynik pomiaru i zaczynam nowy pomiar

	while (1) {

		if (time_out) {
			time_out = 0;
			USART_Transmit('T');
			USART_Transmit('\n');
			USART_Transmit('\n');
			make_measure();
		}

		if (jestPomiar) {
			jestPomiar = 0;
			USART_Transmit('P');
			USART_Transmit('\n');
			show_timer_counter(); 	//wyswietlam wartowsc licznika
			make_measure();
		}
	}

	return 0;
}

ISR(INT0_vect) {					//zmieniony został stan naa pinie echo
	current_state = PIND & 0b00000100;

	//----------------------rozpoczęcie pomiaru---------------
	if (current_state == 0x04) {
		TCNT1 = 0;					//zeruj licznik
		PORTB &= (0 << dioda);		//włącz diode na czas pomiaaru
	}

	//----------------------zakończenie pomiaru---------------
	else {
		timer = TCNT1H | TCNT1L;	//odczytaj wartosc z licznika
		jestPomiar = 1;				//ustaw flage zakończenia pomiaru
		PORTB |= (1 << PB2);		//wyłącz diode
	}
}

ISR(TIMER1_OVF_vect) {
	PORTB |= (1 << PB2);			//wyłącz diode
	time_out = 1;					//ustaw flage zbyt długiego czasu pomiaru
}

/*						POWSTAŁE PROBLEMY
 * - Nie działa receiver
 * Nie następuje odczytanie zbocza opadającego sygnalizującego
 * zakończenie pomiaru (kabel ???) W przypadku poruszania kablami wysypuje
 * danymi na UART, jest to kwestia nie łacznia kabelka, czy mechanicznego
 * wywołania zmiany stanu na pinie?
 */
