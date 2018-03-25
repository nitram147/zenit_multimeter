/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                 display.h                  | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#ifndef DISPLAY_H
#define DISPLAY_H

#define DEGREE_CHARACTER 223
#define OHM_CHARACTER 244

volatile uint8_t lcd_fix_cursor_position;
void lcd_puts_fixed(char *tmp_string);
void lcd_putc_fixed(char tmp_char);

typedef enum{
	U_MEASUREMENT, I_MEASUREMENT, R_MEASUREMENT,
	D_MEASUREMENT, T_MEASUREMENT, WIFI_MODE
} DISPLAY_MODE_T;

typedef enum{
	SHOW_LABELS, HIDE_LABELS
} LABELS_STATE_T;

DISPLAY_MODE_T display_interface_mode;

LABELS_STATE_T labels_state;

uint8_t display_need_to_be_rewrited;

void erase_display();

void print_ip_address();

void lcd_setup_position(uint8_t tmp_x, uint8_t tmp_y);

void custom_char_wifi_init();

void show_custom_wifi_char();

void display_actual_mode_content(uint8_t tmp_measure_it);

void display_range(MEASURING_RANGES_T tmp_requested, MEASURING_RANGES_T tmp_actual);

void display_number_with_unit_prefix(float tmp_number, uint8_t tmp_precision);

void mode_u_measurement(uint8_t tmp_measure_it);

void mode_i_measurement(uint8_t tmp_measure_it);

void mode_r_measurement(uint8_t tmp_measure_it);

void mode_d_measurement(uint8_t tmp_measure_it);

void mode_t_measurement(uint8_t tmp_measure_it);

void mode_wifi();

#endif