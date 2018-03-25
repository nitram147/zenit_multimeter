/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                   esp.h                    | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#ifndef ESP_H
#define ESP_H

//wifi credentials
#define WIFI_SSID "your_wifi"
#define WIFI_PASS "your_password"

//port on which will device listen to SCPI commands
#define SCPI_PORT "1234"

//wifi enable resistor (74hc125d enable pin) settings
#define WIFI_ENABLE_DDR DDRD
#define WIFI_ENABLE_PORT PORTD
#define WIFI_ENABLE_BIT PD2

//max time for waiting to execution status change
//time is calculated as time[s] = MAX_ITERATIONS_TO_WAIT*50/1000;
#define MAX_ITERATIONS_TO_WAIT 350

typedef struct {
	uint8_t first;
	uint8_t second;
	uint8_t third;
	uint8_t fourth;
} IP_ADDRESS_T;

typedef struct {
	uint8_t channel;
	char *message;
} TCP_MESSAGE_T;

typedef enum { WAITING_TO_EXECUTE, EXECUTED_OK, EXECUTED_FAIL, EXECUTED_ERROR } EXECUTION_STATUS_T;

typedef struct{
	uint8_t wifi_enabled;
	uint8_t connected_to_wifi;
	uint8_t wifi_got_ip;
	EXECUTION_STATUS_T last_command_executed_successfuly;
	IP_ADDRESS_T ip_address;
} ESP_STATE_T;

ESP_STATE_T esp_state;

extern void send_string(char *s, uint8_t tmp_crln);

extern uint8_t is_number(char tmp_char);

extern uint8_t char_to_digit(char tmp_char);

extern void erase_display();
extern void lcd_puts_fixed(char *tmp_string);

void enable_wifi();

void disable_wifi();

void wait_until_executed(char *tmp_string_to_write);

void send_to_esp(char *tmp_string);

void connect_to_wifi();

void enable_server();

void parse_ip_address(char* tmp_string, IP_ADDRESS_T* tmp_where);

void esp_init();

void send_tcp_message(uint8_t tmp_channel, char *tmp_string);

#endif