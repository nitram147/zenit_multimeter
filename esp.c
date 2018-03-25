/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                   esp.c                    | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "esp.h"
#include "uart.h"
#include "parser.h"
#include "lcd.h"

ESP_STATE_T esp_state = {0,0,0,0,{0,0,0,0}};

void enable_wifi(){
	WIFI_ENABLE_DDR |= (1<<WIFI_ENABLE_BIT);	//set enable pin as output
	WIFI_ENABLE_PORT &=~(1<<WIFI_ENABLE_BIT);	//pull enable pin down (74hc125d enabled on low)

	esp_state.wifi_enabled = 1;
	_delay_ms(50);
}

void disable_wifi(){
	WIFI_ENABLE_DDR |= (1<<WIFI_ENABLE_BIT);	//set enable pin as output
	WIFI_ENABLE_PORT |= (1<<WIFI_ENABLE_BIT);	//pull enable pin up (74hc125d disabled on high)

	esp_state.wifi_enabled = 0;
	esp_state.connected_to_wifi = 0;
	esp_state.wifi_got_ip = 0;
	
	_delay_ms(50);
}

//wait until last command was executed
void wait_until_executed(char *tmp_string_to_write){

	erase_display();
	lcd_puts_fixed(tmp_string_to_write);

	//max time for waiting to execution status change
	//time is calculated as time[s] = tmp_max_iterations*50/1000;
	uint16_t tmp_max_iterations = MAX_ITERATIONS_TO_WAIT;

	while(esp_state.last_command_executed_successfuly == WAITING_TO_EXECUTE){

		_delay_ms(50);
		//if we have reached maximum waiting time, break waiting loop
		//this situation shouldn't happen but in case it happens we don't
		//want to get stucked in infinity loop
		if(!(--tmp_max_iterations)) break;

	}

	esp_state.last_command_executed_successfuly = WAITING_TO_EXECUTE;
	lcd_clrscr();lcd_home();
}

void send_to_esp(char *tmp_string){
	//end string with CRLF
	send_string(tmp_string, 1);
}

void connect_to_wifi(){
	send_to_esp("AT+CWJAP=\""WIFI_SSID"\",\""WIFI_PASS"\"");
	wait_until_executed("ConnectingToWifi");
}

void obtain_ip_address(){
	send_to_esp("AT+CIFSR");
	wait_until_executed("Waiting for IP");
}

void enable_server(){
	send_to_esp("AT+CIPMUX=1");
	wait_until_executed("Cipmux setup");

	send_to_esp("AT+CIPSERVER=1,"SCPI_PORT);
	wait_until_executed("Enabling server");
}

void parse_ip_address(char* tmp_string, IP_ADDRESS_T* tmp_where){

	char tmp_char=0;
	//current IP octet position (1-4), set to 0 until we hit first double quote (start of IP)
	uint8_t tmp_position=0;

	//set IP to 0.0.0.0
	tmp_where->first=0;
	tmp_where->second=0;
	tmp_where->third=0;
	tmp_where->fourth=0;

	//until we had string, load first character and increase pointer to next position
	while((tmp_char = *tmp_string++) != 0x00){

		if(is_number(tmp_char)){

			if(tmp_position == 1){

				tmp_where->first *= 10;
				tmp_where->first += char_to_digit(tmp_char);

			}else if(tmp_position == 2){

				tmp_where->second *= 10;
				tmp_where->second += char_to_digit(tmp_char);

			}else if(tmp_position == 3){

				tmp_where->third *= 10;
				tmp_where->third += char_to_digit(tmp_char);	

			}else if(tmp_position == 4){

				tmp_where->fourth *= 10;
				tmp_where->fourth += char_to_digit(tmp_char);
			}

		//when IP had stared and we hit '.' octet separator
		}else if(tmp_char == '.' && tmp_position != 0){
			//increase octet position
			if(tmp_position<4){tmp_position++;}

		}else if(tmp_char == '"'){
			//if we hit first double quote, set position to first octet of IP
			if(tmp_position == 0){
				tmp_position = 1;
			//if we hit second double quote, IP had ended, return function
			}else{
				return;
			}
		
		//when IP already started and we hit unknown char, return function
		}else if(tmp_position != 0){
			return;
		}

	}
}

void esp_init(){

	enable_wifi();

	//esp init
	send_to_esp("AT");
	wait_until_executed("AT");

	//restart esp
	send_to_esp("AT+RST");
	wait_until_executed("Resetting ESP");

	send_to_esp("ATE0");
	wait_until_executed("Disabling echo");

	//set esp mode to STATION
	send_to_esp("AT+CWMODE=1");
	wait_until_executed("StationModeSetup");

	_delay_ms(500);

	connect_to_wifi();

	enable_server();

	obtain_ip_address();
}

void send_tcp_message(uint8_t tmp_channel, char *tmp_string){
	uint8_t tmp_length;
	char tmp_buffer[16];

	//calculate message length (length of message + 2 bytes (CRLN))
	tmp_length = strlen(tmp_string) + 2;
	//send string without (CRLN)
	send_string("AT+CIPSEND=",0);
	
	//convert channel number to string and send it (without CRLN)
	itoa(tmp_channel, tmp_buffer,10);
	send_string(tmp_buffer,0);
	
	send_string(",",0);

	//convert message length to string and sent it (again without CRLN)
	itoa(tmp_length, tmp_buffer, 10);
	send_string(tmp_buffer, 1);

	//wait for esp to respond
	_delay_ms(30);
	//send message with CRLN ending
	send_string(tmp_string, 1);

}