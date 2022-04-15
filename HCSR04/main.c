/*
 * HCSR04.c
 *
 * Created: 28-05-2020 12.36.43 PM
 * Author : The Broddring Empire Special Academics Division, Ministry of Academics.
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#define BAUDRATE 9600
#define BAUD_PRESCALLER F_CPU/16/BAUDRATE-1

volatile long int duration; // stores TCNT1 value. The duration for which the counter was ON, i.e. from the instant the echo pin went high to that instant when it went low.
volatile int distance;
char buffer [5];

void USART_init(void)
{	
	UBRR0H = (uint8_t)((BAUD_PRESCALLER)>> 8);     
	UBRR0L = (uint8_t)(BAUD_PRESCALLER);          
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);               
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);             
}

void USART_send(uint8_t data)
{
	while(!(UCSR0A & (1<<UDRE0)));                
	UDR0 = data;                                  	
}

void printString(const char myString[]) 
{
	uint8_t i = 0;
	while (myString[i]) 
	{
		USART_send(myString[i]);
		i++;
	}
}

void initInterrupt0(void) // Configure Interrupt
{
	EIMSK |= (1 << INT0); // enable INT0 
	EICRA |= (1 << ISC11); // The falling edge on INT0 generates an interrupt request i.e., when the echo pin goes LOW, the counter has to stop counting and store the count value.
	sei(); //set (global) interrupt enable bit 
}

ISR(INT0_vect) // Interrupt Service Routine triggered on falling edge of Echo pin.
{
	/* The echo pin goes high as soon as ultrasound wave is sent and it goes low only when it receives the reflected wave or when timed out. 
	The duration for which the echo pin stays high is directly proportional to the distance of the obstacle from sensor. */
	TCCR1B = 0; // Stop counter
	duration = TCNT1; // Store counter value (duration).
	TCNT1 = 0; // Reset counter.
}

int main(void)
{
	DDRD |= (1 << 3); // PD3 Trigger pin (INT1).
	DDRD &= ~(1 << 2);// PD2 Echo pin (INT0).
	PORTD |= (1 << 2);// Enable pull up.
	USART_init();
	initInterrupt0();
    while (1) 
    {
		TCCR1B |= (1<<CS11)|(1<<CS10);/* Echo pin is assumed to go high as soon as the trigger pin goes high. So the counter is started by setting the Prescaler value, as soon
                                          as trigger pin goes high, so that it is able to determine the exact duration for which the echo pin stays high */
		PORTD &= ~(1 << 3);
		_delay_us(10); // In the loop first you have to make sure that the trigPin is clear so we have to set that pin on a LOW State for just 10 us. 
		PORTD |= (1 << 3);
		_delay_us(10); // Now for generating the ultrasound wave we have to set the trigPin on HIGH State for 10 us. 
		PORTD &= ~(1 << 3); 
		_delay_us(10);
		distance = duration*0.034/2; 
		itoa(distance,buffer,10);
		printString(buffer);
		printString("\r\n");
    }
}