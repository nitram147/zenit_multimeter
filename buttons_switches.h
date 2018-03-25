/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |             buttons_switches.h             | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#ifndef BUTTONS_SWITCHES_H
#define BUTTONS_SWITCHES_H

//raw ADC value when button pressed (only one specific button at time)
#define BUTTON_1_ADC 120
#define BUTTON_2_ADC 230
#define BUTTON_3_ADC 335
#define BUTTON_4_ADC 440
#define BUTTON_5_ADC 560
#define BUTTON_6_ADC 690
#define BUTTON_7_ADC 840
#define BUTTON_8_ADC 1020

#define BUZZER_DDR DDRD
#define BUZZER_PORT PORTD
#define BUZZER_BIT PD5

uint16_t actual_buttons_colors;

typedef enum {
	NO_BUTTON, FIRST_BUTTON, SECOND_BUTTON,
	THIRD_BUTTON, FOURTH_BUTTON, FIFTH_BUTTON,
	SIXTH_BUTTON, SEVENTH_BUTTON, EIGHT_BUTTON
} BUTTONS_T;

typedef enum { NO_COLOR, RED, GREEN } COLORS_T;

typedef enum {
	FE_RES_REF = 0b00000000,
	FE_RES_MEAS = 0b01000000,
	FE_I40MA = 0b10000000,
	FE_I400MA = 0b10000000,
	FE_I5A = 0b10100000, 		//DC current
	FE_UDC = 0b10110000,		//DC voltage, all switches open
	FE_UAC = 0b11110000,		//AC voltage, all switches open
	FE_GAIN2 = 0b00000000,		//Gain x2 switch
	FE_GAIN46 = 0b00001000,		//Gain x46 switch
	FE_UDIV2 = 0b00000000,		//U/2 no switches closed
	FE_UDIV20 = 0b00000100,		//U/20 switch closed
	FE_UDIV200 = 0b00000010,	//U/200 switch closed
} SWITCHES_T;

BUTTONS_T read_buttons();

void set_buttons(uint16_t tmp_buttons);

void set_switches(SWITCHES_T tmp_switches);

void change_button_color(BUTTONS_T tmp_which_button, COLORS_T tmp_color);

void buttons_init();

void clear_measurements_modes_buttons_colors();

void evaluate_buttons();

void button_1_pressed();

void button_2_pressed();

void button_3_pressed();

void button_4_pressed();

void button_5_pressed();

void button_6_pressed();

void button_7_pressed();

void button_8_pressed();

#endif