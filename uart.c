/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                   uart.c                   | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "uart.h"

uint8_t count_of_arrived = 0;
uint8_t discard_remaining = 0;

//enable UART, enable uart byte received interrupt and enable interrupts 
void enable_uart(){
	//uart settings
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
	UBRR0H = (BAUD_PRESCALE>>8);
	UBRR0L = BAUD_PRESCALE;

	//enable interrupts
	sei();
	//enable uart byte received interrupt
	UCSR0B |= (1<<RXCIE0);
}

//send one byte(char) to uart
void send_char(char tmp_char){
	UDR0 = tmp_char;
	while((UCSR0A&(1<<UDRE0))==0){};
}

//send string to uart, end string with carriage return and new line (optional via tmp_crln parameter)
void send_string(char *s, uint8_t tmp_crln){
   while (*s != 0x00){
   		send_char(*s++);
   	}

   if(tmp_crln){
   		send_char('\r');send_char('\n');
   	}
}

//routine called by uart byte received interrupt
ISR(USART_RX_vect){

	if(!discard_remaining){

		arrived_buffer[count_of_arrived] = UDR0;

		//if we hit end of string (CR or LN)
		if(arrived_buffer[count_of_arrived] == '\r' || arrived_buffer[count_of_arrived] == '\n'){
			
			//end string with null-byte
			arrived_buffer[count_of_arrived] = 0x00;

			count_of_arrived = 0;
			discard_remaining = 1;

			//copy arrival buffer to completed buffer
			strcpy(arrived_completed, arrived_buffer);
			string_arrived(arrived_completed);

		}else{

			//increase buffer index (when we have enough space remaining)
			if(count_of_arrived < BUFFER_MAX_INDEX) count_of_arrived++;
			
		}

	}else{

		//load char to discard
		char tmp_discard = UDR0;
		//if the char isn't end of line, save it (it's first char of next string)
		if(tmp_discard != '\r' && tmp_discard != '\n'){
			//save new char to buffer
			arrived_buffer[count_of_arrived] = tmp_discard;
			
			//reset discard flag, increase buffer index
			discard_remaining=0;
			count_of_arrived++;
			
		}
	}

}