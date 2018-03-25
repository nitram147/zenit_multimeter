/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                  parser.h                  | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#ifndef PARSER_H
#define PARSER_H

typedef enum {

	MEASURE_RES, MEASURE_CURR, MEASURE_VOLT, MEASURE_RAW,
	MEASURE_DIOD, MEASURE_TEMP_RTD, MEASURE_TEMP_NTC,

	CHECK_MEASURE_VOLT_RANGE, CHECK_MEASURE_CURR_RANGE,

	SET_MEASURE_VOLT_RANGE, SET_MEASURE_CURR_RANGE,

	UNKNOWN_COMMAND,
	
	CHECK_IDN,

	CHECK_CAL_VREF,

	CHECK_CAL_VOLT_U4_SLOPE, CHECK_CAL_VOLT_U4_OFFSET,
	CHECK_CAL_VOLT_U40_SLOPE, CHECK_CAL_VOLT_U40_OFFSET,
	CHECK_CAL_VOLT_U400_SLOPE, CHECK_CAL_VOLT_U400_OFFSET,

	CHECK_CAL_CURR_I5_SLOPE, CHECK_CAL_CURR_I5_OFFSET,
	CHECK_CAL_CURR_I40_SLOPE, CHECK_CAL_CURR_I40_OFFSET,
	CHECK_CAL_CURR_I400_SLOPE, CHECK_CAL_CURR_I400_OFFSET,

	CHECK_CAL_REFERENCE_R1, CHECK_CAL_REFERENCE_R2,

	CHECK_CAL_TEMP_RTD_R0, CHECK_CAL_TEMP_RTD_COEFF_A,
	CHECK_CAL_TEMP_NTC_R25, CHECK_CAL_TEMP_NTC_COEFF_B,

	SET_CAL_VREF,

	SET_CAL_VOLT_U4_SLOPE, SET_CAL_VOLT_U4_OFFSET,
	SET_CAL_VOLT_U40_SLOPE, SET_CAL_VOLT_U40_OFFSET,
	SET_CAL_VOLT_U400_SLOPE, SET_CAL_VOLT_U400_OFFSET,

	SET_CAL_CURR_I5_SLOPE, SET_CAL_CURR_I5_OFFSET,
	SET_CAL_CURR_I40_SLOPE, SET_CAL_CURR_I40_OFFSET,
	SET_CAL_CURR_I400_SLOPE, SET_CAL_CURR_I400_OFFSET,

	SET_CAL_REFERENCE_R1, SET_CAL_REFERENCE_R2,

	SET_CAL_TEMP_RTD_R0, SET_CAL_TEMP_RTD_COEFF_A,
	SET_CAL_TEMP_NTC_R25, SET_CAL_TEMP_NTC_COEFF_B,

} COMMANDS_T;

typedef enum { RANGE_1, RANGE_2, RANGE_3, RANGE_AUTO, UNKNOWN_RANGE } MEASURING_RANGES_T;

typedef enum { RTD, NTC } TEMPERATURE_SENSOR_TYPE_T;

MEASURING_RANGES_T voltage_range_requested;
MEASURING_RANGES_T current_range_requested;

MEASURING_RANGES_T voltage_range_actual;
MEASURING_RANGES_T current_range_actual;

TEMPERATURE_SENSOR_TYPE_T selected_temperature_sensor_type;

typedef struct{
	COMMANDS_T command;
	float argument;
	MEASURING_RANGES_T range;
} PARSER_OUTPUT_T;

enum message_sources {	ESP, USB };

typedef struct{
	enum message_sources from;
	uint8_t channel;
} MESSAGE_SOURCE_T;

uint8_t is_number(char tmp_char);

uint8_t char_to_digit(char tmp_char);

uint8_t compare_strings(char *tmp_first, char *tmp_second);

uint8_t compare_string_to_lenght(char *tmp_first, char *tmp_second, uint8_t tmp_lenght);

uint8_t does_string_start_with(char *tmp_string, char *tmp_start);

char *remove_to_separator(char *tmp_string, char tmp_separator);

char *remove_before_separator(char *tmp_string, char tmp_separator);

char *remove_string_before_separators(char *tmp_string, char* tmp_separators, uint8_t tmp_separators_count);

char *remove_string_from_string(char *tmp_string, char *tmp_to_remove);

MEASURING_RANGES_T string_to_range(char *tmp_string);

void range_to_string(MEASURING_RANGES_T tmp_range, char *tmp_buffer);

void float_to_string(float tmp_number, char *tmp_buffer);

float string_to_float(char *tmp_string);

void esp_unknown_command(uint8_t tmp_channel);

void unknown_command();

void send_message_to_destination(char *tmp_message, MESSAGE_SOURCE_T tmp_destination);

void scpi_command_executor(PARSER_OUTPUT_T tmp_parsed_command, MESSAGE_SOURCE_T tmp_destination);

PARSER_OUTPUT_T scpi_command_parser(char *tmp_string);

void esp_command_arrived(TCP_MESSAGE_T tmp_message);

void usb_command_arrived(char *tmp_string);

void string_arrived(char *tmp_string);

#endif