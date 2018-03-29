/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                 display.c                  | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#include <avr/io.h>
#include <stdlib.h>

#include "esp.h" //parser.h require definitions from esp.h
#include "parser.h" //definition of measuring_ranges and global variables
#include "display.h"
#include "lcd.h"
#include "buttons_switches.h"
#include "measure.h"

//global variable containing selected mode of device
DISPLAY_MODE_T display_interface_mode = U_MEASUREMENT;

//global variable used to determine whether show or no labels before measured values
//label example "U="
LABELS_STATE_T labels_state = SHOW_LABELS;

//flag which indicate if we need to rewrie display
//because we don't want to rewrite display each time
uint8_t display_need_to_be_rewrited = 1;

//lcd puts function fix
//because 16x1 display is working like 8x2 display
volatile uint8_t lcd_fix_cursor_position = 0;
void lcd_puts_fixed(char *tmp_string){
	char tmp_char;

	while((tmp_char = *tmp_string++) != 0x00){
		if(lcd_fix_cursor_position == 8) lcd_putc('\n');
		lcd_putc(tmp_char);
		lcd_fix_cursor_position++;
	}

}

//lcd putc function fix
//because 16x1 display is working like 8x2 display
void lcd_putc_fixed(char tmp_char){
	if(lcd_fix_cursor_position == 8) lcd_putc('\n');
	lcd_putc(tmp_char);
	lcd_fix_cursor_position++;
}

//erase display and go to home position
void erase_display(){
	lcd_clrscr();lcd_home();
	lcd_fix_cursor_position = 0;
}

//print IP address assigned to ESP
void print_ip_address(){
	char tmp_buffer[4];

	//convert & print first octet
	itoa(esp_state.ip_address.first,tmp_buffer,10);
	lcd_puts_fixed(tmp_buffer);lcd_putc_fixed('.');

	//convert & print second octet
	itoa(esp_state.ip_address.second,tmp_buffer,10);
	lcd_puts_fixed(tmp_buffer);lcd_putc_fixed('.');

	//convert & print third octet
	itoa(esp_state.ip_address.third,tmp_buffer,10);
	lcd_puts_fixed(tmp_buffer);lcd_putc_fixed('.');

	//convert & print fourth octet
	itoa(esp_state.ip_address.fourth,tmp_buffer,10);
	lcd_puts_fixed(tmp_buffer);
}

//setup lcd position before writing custom char
void lcd_setup_position(uint8_t tmp_x, uint8_t tmp_y){
	
	if(tmp_y == 0) lcd_command(0x80 | tmp_x);
	else if(tmp_y == 1)	lcd_command(0xC0 | tmp_x);
}

//upload custom char to CGRAM of display driver
void custom_char_wifi_init(){

	lcd_command(0x40);
	lcd_data(0b00000000);
	lcd_data(0b00011111);
	lcd_data(0b00010000);
	lcd_data(0b00010111);
	lcd_data(0b00010100);
	lcd_data(0b00010101);
	lcd_data(0b00000000);
	lcd_data(0b00000000);

}

//show custom char (wifi symbol) on the last display position
void show_custom_wifi_char(){

	//display custom char only if we are successfully connected to wifi
	if(esp_state.connected_to_wifi && esp_state.wifi_got_ip){
		//upload custom char to CGRAM
		custom_char_wifi_init();
		//setup lcd position before writing custom char
		lcd_setup_position(7,1);
		//write first custom character stored in CGRAM (our custom wifi symbol)
		lcd_data(0);
	}

}

//call specific display function based on currently selected mode of device
void display_actual_mode_content(uint8_t tmp_measure_it){

	switch(display_interface_mode){

		case U_MEASUREMENT: mode_u_measurement(tmp_measure_it); break;

		case I_MEASUREMENT: mode_i_measurement(tmp_measure_it); break;

		case R_MEASUREMENT: mode_r_measurement(tmp_measure_it); break;

		case D_MEASUREMENT: mode_d_measurement(tmp_measure_it); break;

		case T_MEASUREMENT: mode_t_measurement(tmp_measure_it); break;

		///* reserved for sixth button mode */case (): break;

		///* reserved for seventh button mode */case (): break;

		case WIFI_MODE: mode_wifi(); break;

	}
}

void display_range(MEASURING_RANGES_T tmp_requested, MEASURING_RANGES_T tmp_actual){

	//go to desired location
	//(fourth character, second line) - because display is behaving like 8x2
	lcd_gotoxy(4,1);

	if(tmp_requested == RANGE_AUTO) lcd_putc_fixed('A');
	else lcd_putc_fixed('M');

	//increase range number by one (because we are indexing enum from 0)
	//add '0' ascii value to convert number to character
	lcd_putc_fixed(tmp_actual+1+'0');
	
}

void display_number_with_unit_prefix(float tmp_number, uint8_t tmp_precision){

	char tmp_unit_prefix = 0;
	char tmp_buffer[16];

	uint8_t tmp_count_of_decimals;

	//in case of negative number
	if(tmp_number < 0){
		//display negative sign
		lcd_putc_fixed('-');
		//make positive number from negative
		tmp_number = -tmp_number;
	//in case of positive number
	}else{
		//display space
		lcd_putc_fixed(' ');
	}

	//if our number is smaller than one (unit prefix mili)
	if(tmp_number < 1.0000){

		tmp_unit_prefix = 'm';

		if(tmp_number < 0.0100) tmp_count_of_decimals = 4;
		else if(tmp_number < 0.1000) tmp_count_of_decimals = 3;
		else tmp_count_of_decimals = 2;

		tmp_number *= 1000;

	//if our number is smaller than thousand (unit prefix none)
	}else if(tmp_number < 1000.0){

		if(tmp_number < 10.000) tmp_count_of_decimals = 4;
		else if(tmp_number < 100.00) tmp_count_of_decimals = 3;
		else tmp_count_of_decimals = 2;

	//if our number is smaller than milion (unit prefix kilo)
	}else if(tmp_number < 1000000.0){

		tmp_unit_prefix = 'k';

		if(tmp_number < 10000.0) tmp_count_of_decimals = 4;
		else if(tmp_number < 100000.0) tmp_count_of_decimals = 3;
		else tmp_count_of_decimals = 2;

		tmp_number /= 1000;

	//if our number is bigger than milion (unit prefix mega) 
	}else{

		tmp_unit_prefix = 'M';

		if(tmp_number < 10000000.0) tmp_count_of_decimals = 4;
		else if(tmp_number < 100000000.0) tmp_count_of_decimals = 3;
		else tmp_count_of_decimals = 2;

		tmp_number /= 1000000;
	}


	//increase or decrease count of decimals in case that function is called with different precision
	//default precision is 6, so constants are calculated for that precisions which means
	//they need to be changed in case of different precision
	tmp_count_of_decimals += (tmp_precision - 6);

	//convert number into string with specified precision, output strlen = tmp_precision
	dtostrf(tmp_number, tmp_precision, tmp_count_of_decimals, tmp_buffer);

	//display number with unit prefix
	lcd_puts_fixed(tmp_buffer);
	if(tmp_unit_prefix) lcd_putc_fixed(tmp_unit_prefix);

}

void mode_u_measurement(uint8_t tmp_measure_it){
	
	if(tmp_measure_it) measure_voltage();

	if(display_need_to_be_rewrited){

		clear_measurements_modes_buttons_colors();
		change_button_color(FIRST_BUTTON, GREEN);

		erase_display();

		if(labels_state == SHOW_LABELS) lcd_puts_fixed("U=");

		if(!current_measured_values.overflow){

			display_number_with_unit_prefix(current_measured_values.voltage, ((labels_state == SHOW_LABELS)?6:8));
			lcd_putc_fixed('V');

		}else {lcd_puts_fixed(" OverFlow");}

		display_need_to_be_rewrited = 0;
		
		display_range(voltage_range_requested, voltage_range_actual);
		show_custom_wifi_char();
	}

}

void mode_i_measurement(uint8_t tmp_measure_it){

	if(tmp_measure_it) measure_current();

	if(display_need_to_be_rewrited){

		clear_measurements_modes_buttons_colors();
		change_button_color(SECOND_BUTTON, GREEN);

		erase_display();

		if(labels_state == SHOW_LABELS) lcd_puts_fixed("I=");

		if(!current_measured_values.overflow){

			display_number_with_unit_prefix(current_measured_values.current, ((labels_state == SHOW_LABELS)?6:8));
			lcd_putc_fixed('A');

		}else {lcd_puts_fixed(" OverFlow");}	

		display_need_to_be_rewrited = 0;

		display_range(current_range_requested, current_range_actual);
		show_custom_wifi_char();
	}

}

void mode_r_measurement(uint8_t tmp_measure_it){

	if(tmp_measure_it) measure_resistance();

	if(display_need_to_be_rewrited){

		clear_measurements_modes_buttons_colors();
		change_button_color(THIRD_BUTTON, GREEN);

		erase_display();
		if(labels_state == SHOW_LABELS) lcd_puts_fixed("R=");

		if(current_measured_values.resistance <= MAX_RESISTANCE){

			display_number_with_unit_prefix(current_measured_values.resistance, ((labels_state == SHOW_LABELS)?6:8));
			lcd_putc_fixed(OHM_CHARACTER);

		}else{

			lcd_puts_fixed(" Open");

		}

		display_need_to_be_rewrited = 0;

		show_custom_wifi_char();
	}

}

void mode_d_measurement(uint8_t tmp_measure_it){

	if(tmp_measure_it) measure_diode();

	if(display_need_to_be_rewrited){

		clear_measurements_modes_buttons_colors();
		change_button_color(FOURTH_BUTTON, GREEN);

		erase_display();
		if(labels_state == SHOW_LABELS) lcd_puts_fixed("D=");

		if(current_measured_values.diode < MAX_DIODE_VOLTAGE){

			display_number_with_unit_prefix(current_measured_values.diode, ((labels_state == SHOW_LABELS)?6:8));
			lcd_putc_fixed('V');

		}else{
			
			lcd_puts_fixed(" OverRange");

		}

		display_need_to_be_rewrited = 0;

		show_custom_wifi_char();
	}

}

void mode_t_measurement(uint8_t tmp_measure_it){

	if(tmp_measure_it) measure_temperature();

	if(display_need_to_be_rewrited){

		clear_measurements_modes_buttons_colors();
		change_button_color(FIFTH_BUTTON, GREEN);

		erase_display();
		if(labels_state == SHOW_LABELS) lcd_puts_fixed("T=");

		display_number_with_unit_prefix(current_measured_values.temperature, ((labels_state == SHOW_LABELS)?6:8));

		lcd_putc_fixed(DEGREE_CHARACTER); lcd_putc_fixed('C');

		if(selected_temperature_sensor_type == RTD)	lcd_puts_fixed(" RTD");
		else lcd_puts_fixed(" NTC");

		display_need_to_be_rewrited = 0;

		show_custom_wifi_char();
	}

}

void mode_wifi(){
	
	if(display_need_to_be_rewrited){

		clear_measurements_modes_buttons_colors();
		display_need_to_be_rewrited = 0;

	}
}