/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                   main.c                   | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

#include "lcd.h"
#include "uart.h"
#include "esp.h"
#include "parser.h"
#include "calibration.h"
#include "spi.h"
#include "buttons_switches.h"
#include "measure.h"
#include "display.h"

#define WELCOME_TEXT "Zenit Multimeter"

void after_start(){

	//initialize LCD display
	lcd_init(LCD_DISP_ON);
	//retrieve calibration constants
	init_calibration_constants();
	//enable serial communication
	enable_uart();
	//itialize SPI bus
	spi_init();
	//initialize buttons ADC and set buttons color to none
	buttons_init();

	//display welcome text and wait a little bit
	lcd_puts_fixed(WELCOME_TEXT);
	_delay_ms(2000);

}

int main(){

	//wait until device get to the steady state
	_delay_ms(500);
	//run after start routines
	after_start();

	//because we don't want to do measurement in each iteration of infinity loop
	uint8_t tmp_measure_it = 1;
	uint8_t tmp_count_to_measure = 0;

	//infinity loop
	while(1){

		//set measurement flag to 1 each fifth loop
		if(tmp_count_to_measure > 5){
			tmp_measure_it = 1;
			tmp_count_to_measure = 0;
		}else{
			tmp_measure_it = 0;
			tmp_count_to_measure++;
		}

		//check is any button is pressed and evaluate pertaining function
		evaluate_buttons();
		//display contents pertaining to selected device mode
		display_actual_mode_content(tmp_measure_it);
		_delay_ms(50);

	}
		
	return 0;
}