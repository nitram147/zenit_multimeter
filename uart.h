/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                   uart.h                   | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#ifndef UART_H
#define UART_H

// UART baud rate must be 115200 (it's ESP8266 default baud rate)
#define USART_BAUDRATE 115200
#define BAUD_PRESCALE ((((F_CPU/16)+(USART_BAUDRATE/2))/(USART_BAUDRATE))-1)

//size of char receiving buffer
#define BUFFER_SIZE 128
#define BUFFER_MAX_INDEX (BUFFER_SIZE-1)

char arrived_buffer[BUFFER_SIZE];
char arrived_completed[BUFFER_SIZE];

uint8_t count_of_arrived;
uint8_t discard_remaining;

//pass string to extern function which will parse it
extern void string_arrived(char *tmp_string);

void enable_uart();

void send_char(char tmp_char);

void send_string(char *s, uint8_t tmp_crln);

ISR(USART_RX_vect);

#endif