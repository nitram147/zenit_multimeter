/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |             buttons_switches.c             | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#include <avr/io.h>
#include <util/delay.h>

#include "buttons_switches.h"
#include "spi.h"
#include "esp.h"
#include "parser.h"
#include "display.h"

//global variable to store actual color state of buttons
uint16_t actual_buttons_colors = 0b0000000000000000;

BUTTONS_T read_buttons(){
	//read ADC value
	uint16_t tmp_adc = ADC;

	//find which button was pressed (assuming that at time there's only one button pressed)
	//no button is pressed
	if(tmp_adc <= (BUTTON_1_ADC/2)){
		return NO_BUTTON;

	//first button
	}else if(tmp_adc <= (((BUTTON_2_ADC-BUTTON_1_ADC)/2)+BUTTON_1_ADC)){
		return FIRST_BUTTON;

	//second button
	}else if(tmp_adc <= (((BUTTON_3_ADC-BUTTON_2_ADC)/2)+BUTTON_2_ADC)){
		return SECOND_BUTTON;

	//third button
	}else if(tmp_adc <= (((BUTTON_4_ADC-BUTTON_3_ADC)/2)+BUTTON_3_ADC)){
		return THIRD_BUTTON;

	//fourth button
	}else if(tmp_adc <= (((BUTTON_5_ADC-BUTTON_4_ADC)/2)+BUTTON_4_ADC)){
		return FOURTH_BUTTON;

	//fifth button
	}else if(tmp_adc <= (((BUTTON_6_ADC-BUTTON_5_ADC)/2)+BUTTON_5_ADC)){
		return FIFTH_BUTTON;

	//sixth button
	}else if(tmp_adc <= (((BUTTON_7_ADC-BUTTON_6_ADC)/2)+BUTTON_6_ADC)){
		return SIXTH_BUTTON;

	//seventh button
	}else if(tmp_adc <= (((BUTTON_8_ADC-BUTTON_7_ADC)/2)+BUTTON_7_ADC)){
		return SEVENTH_BUTTON;

	//eight button
	}else{
		return EIGHT_BUTTON;
	}
}

//set actual color state of buttons
void set_buttons(uint16_t tmp_buttons){
	spi_mode0();
	spi_transfer((uint8_t)((tmp_buttons & 0xff00)>>8));
	spi_transfer((uint8_t)(tmp_buttons & 0xff));
	spi_load_pulse();
}

//set switches to desired state
void set_switches(SWITCHES_T tmp_switches){
	spi_mode0();
	spi_transfer((uint8_t)tmp_switches);
	spi_shift_pulse();
}

//change color of specific button
void change_button_color(BUTTONS_T tmp_which_button, COLORS_T tmp_color){
	//clear color of the actual button
	//AND actual colors with inversed bit mask
	//2bits (3) are set on the coresponding button position, buttons start from 0 (reason for -1)
	actual_buttons_colors &=~(3<<((tmp_which_button-1)*2));

	//set color of the actual button
	//OR actual colors with color bit mask on the coresponding position
	actual_buttons_colors |= (tmp_color<<((tmp_which_button-1)*2));

	//apply changes to buttons
	set_buttons(actual_buttons_colors);
}

//initialize buttons ADC
void buttons_init(){
	//because buttons are connected to voltage divider (made of resistors) and read by ADC
	//set AVCC reference, channel 6
	ADMUX = (1<<REFS0)|(1<<MUX2)|(1<<MUX1);
	//enable adc, free-running mode, prescaler 128
	ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADATE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

	//set buzzer pin to low
	BUZZER_DDR |= (1<<BUZZER_BIT);
	BUZZER_PORT &=~(1<<BUZZER_BIT);

	//set buttons colors to default state
	set_buttons(actual_buttons_colors);	
}

//clear color of all buttons excluding wifi button
void clear_measurements_modes_buttons_colors(){
	change_button_color(FIRST_BUTTON, NO_COLOR);
	change_button_color(SECOND_BUTTON, NO_COLOR);
	change_button_color(THIRD_BUTTON, NO_COLOR);
	change_button_color(FOURTH_BUTTON, NO_COLOR);
	change_button_color(FIFTH_BUTTON, NO_COLOR);
	change_button_color(SIXTH_BUTTON, NO_COLOR);
	change_button_color(SEVENTH_BUTTON, NO_COLOR);
}

//check if any button is pressed and evaluate pertaining function
void evaluate_buttons(){

	BUTTONS_T tmp_buttons_state = read_buttons();

	switch(tmp_buttons_state){

		case FIRST_BUTTON: 	
								//wait until button is released
								while(read_buttons() == FIRST_BUTTON);
								button_1_pressed();
								break;

		case SECOND_BUTTON:	
								//wait until button is released
								while(read_buttons() == SECOND_BUTTON);
								button_2_pressed();
								break;

		case THIRD_BUTTON: 	
								//wait until button is released
								while(read_buttons() == THIRD_BUTTON);
								button_3_pressed();
								break;

		case FOURTH_BUTTON:	
								//wait until button is released
								while(read_buttons() == FOURTH_BUTTON);
								button_4_pressed();
								break;

		case FIFTH_BUTTON: 	
								//wait until button is released
								while(read_buttons() == FIFTH_BUTTON);
								button_5_pressed();
								break;

		case SIXTH_BUTTON: 	
								//wait until button is released
								while(read_buttons() == SIXTH_BUTTON);
								button_6_pressed();
								break;

		case SEVENTH_BUTTON:
								//wait until button is released
								while(read_buttons() == SEVENTH_BUTTON);
								button_7_pressed();
								break;

		case EIGHT_BUTTON:
								//wait until button is released
								while(read_buttons() == EIGHT_BUTTON);
								button_8_pressed();
								break;

		case NO_BUTTON: break;

	}
}

// U button pressed
void button_1_pressed(){

	//if we are not in u-measurement mode
	if(display_interface_mode != U_MEASUREMENT){

		display_interface_mode = U_MEASUREMENT;
		display_need_to_be_rewrited = 1;

	//if we are already in u-measurement mode
	}else{

		//increase requested voltage range
		if(voltage_range_requested < RANGE_AUTO){

			voltage_range_requested++;
			if(voltage_range_requested != RANGE_AUTO) voltage_range_actual = voltage_range_requested;

		}else{

			voltage_range_requested = RANGE_1;
			voltage_range_actual == voltage_range_requested;
		}

		display_need_to_be_rewrited = 1;

	}

}

// I button pressed
void button_2_pressed(){

	if(display_interface_mode != I_MEASUREMENT){

		display_interface_mode = I_MEASUREMENT;
		display_need_to_be_rewrited = 1;

	}else{

		//increase requested voltage range
		if(current_range_requested < RANGE_AUTO){

			current_range_requested++;
			if(current_range_requested != RANGE_AUTO) current_range_actual = current_range_requested;
			else current_range_actual = RANGE_1; //because auto range in current measurement mode cannot be in range_3 !!!

		}else{

			current_range_requested = RANGE_1;
			current_range_actual == current_range_requested;
		}

		display_need_to_be_rewrited = 1;

	}
	
}

// R button pressed
void button_3_pressed(){

	if(display_interface_mode != R_MEASUREMENT){

		display_interface_mode = R_MEASUREMENT;
		display_need_to_be_rewrited = 1;

	}
	
}

// D button pressed
void button_4_pressed(){

	if(display_interface_mode != D_MEASUREMENT){

		display_interface_mode = D_MEASUREMENT;
		display_need_to_be_rewrited = 1;

	}
	
}

// T button pressed
void button_5_pressed(){

	if(display_interface_mode != T_MEASUREMENT){

		display_interface_mode = T_MEASUREMENT;
		display_need_to_be_rewrited = 1;

	}else{

		//change type of temperature sensor
		if(selected_temperature_sensor_type == RTD) selected_temperature_sensor_type = NTC;
		else selected_temperature_sensor_type = RTD;

	}

}

void button_6_pressed(){}

//show/hide labels button pressed
void button_7_pressed(){

	//invert label state
	(labels_state == SHOW_LABELS)?(labels_state = HIDE_LABELS):(labels_state = SHOW_LABELS);
	display_need_to_be_rewrited = 1;

}

//wifi button pressed
void button_8_pressed(){

	//if we are not in wifi mode
	if(display_interface_mode != WIFI_MODE){

		//set mode to wifi mode
		display_interface_mode = WIFI_MODE;
		display_need_to_be_rewrited = 1;

		//if wifi isn't enabled yet
		if(!esp_state.wifi_enabled){

			change_button_color(EIGHT_BUTTON, RED);			
			esp_init();

			if(esp_state.connected_to_wifi && esp_state.wifi_got_ip){

				change_button_color(EIGHT_BUTTON, GREEN);
				erase_display(); print_ip_address();

			}else{

				erase_display(); lcd_puts_fixed("Couldn't connect"); _delay_ms(800);

			}			

		//if we enter wifi mode (wifi is already enabled)
		}else{

			erase_display();

			//if we are successfully connected to wifi
			if(esp_state.connected_to_wifi && esp_state.wifi_got_ip){
				print_ip_address();
			//if we aren't connected to wifi or we haven't obtained IP address
			}else{
				lcd_puts_fixed("Wifi has problem");
			}

		}

	//if we are already in wifi mode
	}else{
		
		//if wifi is enabled
		if(esp_state.wifi_enabled){

			disable_wifi();
			change_button_color(EIGHT_BUTTON, NO_COLOR);
			erase_display(); lcd_puts_fixed("Wifi disabled"); _delay_ms(800);

		//if wifi isn't enabled
		}else{

			change_button_color(EIGHT_BUTTON, RED);	

			esp_init();

			//if we are successfully connected to wifi
			if(esp_state.connected_to_wifi && esp_state.wifi_got_ip){

				change_button_color(EIGHT_BUTTON, GREEN);
				erase_display(); print_ip_address();

			//if we aren't connected to wifi or we haven't obtained IP address
			}else{

				erase_display(); lcd_puts_fixed("Couldn't connect"); _delay_ms(800);

			}

		
		}
		
	}


}