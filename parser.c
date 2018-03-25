/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                  parser.c                  | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#include <avr/io.h>
#include <avr/interrupt.h> //because uart library require it
#include <string.h>
#include <stdlib.h>

#include "esp.h"
#include "parser.h"
#include "uart.h"
#include "calibration.h"
#include "measure.h"
#include "display.h"

#define MULTIMETER_IDN "Multimeter ZENIT"

MEASURING_RANGES_T voltage_range_requested = RANGE_AUTO;
MEASURING_RANGES_T current_range_requested = RANGE_AUTO;

MEASURING_RANGES_T voltage_range_actual = RANGE_1;
MEASURING_RANGES_T current_range_actual = RANGE_1;

TEMPERATURE_SENSOR_TYPE_T selected_temperature_sensor_type = RTD;

uint8_t is_number(char tmp_char){
	return (tmp_char >= '0' && tmp_char <= '9');
}

uint8_t char_to_digit(char tmp_char){
	if(is_number(tmp_char)){
		return (tmp_char - '0');
	}else{
		return 0;
	}
}

uint8_t compare_strings(char *tmp_first, char *tmp_second){
	return (strcmp(tmp_first, tmp_second) == 0);
}

uint8_t compare_string_to_lenght(char *tmp_first, char *tmp_second, uint8_t tmp_lenght){
	return (strncmp(tmp_first, tmp_second, tmp_lenght) == 0);
}

uint8_t does_string_start_with(char *tmp_string, char *tmp_start){
	return compare_string_to_lenght(tmp_string, tmp_start, strlen(tmp_start));
}

//remove everything before separator (separator is removed too)
char *remove_to_separator(char *tmp_string, char tmp_separator){
	//remove everything before separator (if the string contain separator)
	while(*tmp_string != tmp_separator && *tmp_string != 0x00){tmp_string++;}

	//if there isn't end of string, remove remaining separator
	if(*tmp_string != 0x00){tmp_string++;}

	return tmp_string;
}

//remove everything before separator (separator is not removed)
char *remove_before_separator(char *tmp_string, char tmp_separator){
	//remove everything before separator (if the string contain separator)
	while(*tmp_string != tmp_separator && *tmp_string != 0x00){tmp_string++;}

	return tmp_string;
}

//remove everything before separator, return pointer to separator or NULL in case we haven't found
//any of the separators
char *remove_string_before_separators(char *tmp_string, char* tmp_separators, uint8_t tmp_separators_count){

	char *tmp_new_string = tmp_string;

	for(uint8_t tmp_i=0; tmp_i < tmp_separators_count; tmp_i++){
		//remove everything before separator (if the string contain separator)
		while(*tmp_new_string != tmp_separators[tmp_i] && *tmp_new_string != 0x00){tmp_new_string++;}

		//in case we found separator, return pointer to it
		if(*tmp_new_string == tmp_separators[tmp_i]){
			return tmp_new_string;
		//in case that we haven't found separator restore pointer to start of string
		}else{
			tmp_new_string = tmp_string;
		}

	}

	//in case that we haven't found any separator return NULL
	return NULL;
	
}

char *remove_string_from_string(char *tmp_string, char *tmp_to_remove){
	// remove x characters from beginning of string
	// x is lenght of string to remove
	for(uint8_t tmp_i=0; tmp_i < strlen(tmp_to_remove); tmp_i++){
		// we will increase pointer only if we still haven't reached end of string
		if(*tmp_string != 0x00){tmp_string++;}
	}

	return tmp_string;
}

MEASURING_RANGES_T string_to_range(char *tmp_string){

	if(compare_strings(tmp_string, "1")){
		return RANGE_1;
	}else if(compare_strings(tmp_string, "2")){
		return RANGE_2;
	}else if(compare_strings(tmp_string, "3")){
		return RANGE_3;
	}else if(compare_strings(tmp_string, "AUTO")){
		return RANGE_AUTO;
	}

	return UNKNOWN_RANGE;
}

void range_to_string(MEASURING_RANGES_T tmp_range, char *tmp_buffer){
	if(tmp_range == RANGE_1){
		strcpy(tmp_buffer, "1");
	}else if(tmp_range == RANGE_2){
		strcpy(tmp_buffer, "2");
	}else if(tmp_range == RANGE_3){
		strcpy(tmp_buffer, "3");
	}else if(tmp_range == RANGE_AUTO){
		strcpy(tmp_buffer, "AUTO");
	}
}

void float_to_string(float tmp_number, char *tmp_buffer){
	dtostre(tmp_number, tmp_buffer, 12, 8);
}

float string_to_float(char *tmp_string){
	//parameters (string, pointer to end of number (not used in case of NULL))
	return strtod(tmp_string, NULL);
}

//send unknown command message over TCP
void esp_unknown_command(uint8_t tmp_channel){
	send_tcp_message(tmp_channel, "Error: Unknown command");
}

//send unknown message on UART
void unknown_command(){
	send_string("Error: Unknown command",1);
}

void send_message_to_destination(char *tmp_message, MESSAGE_SOURCE_T tmp_destination){

	if(tmp_destination.from == ESP){
		send_tcp_message(tmp_destination.channel, tmp_message);
	}else if(tmp_destination.from == USB){
		send_string(tmp_message,1);
	}

}

void scpi_command_executor(PARSER_OUTPUT_T tmp_parsed_command, MESSAGE_SOURCE_T tmp_destination){

	char *tmp_message_to_send = "";
	char tmp_buffer[16];

	if(tmp_parsed_command.command == UNKNOWN_COMMAND){
		tmp_message_to_send = "Error: Unknown command";

	}else if(tmp_parsed_command.command == CHECK_IDN){
		tmp_message_to_send = MULTIMETER_IDN;

	}else if(tmp_parsed_command.command == MEASURE_RES){
		float_to_string(measure_resistance(), tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == MEASURE_CURR){
		float_to_string(measure_current(), tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == MEASURE_VOLT){
		float_to_string(measure_voltage(), tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == MEASURE_RAW){
		float_to_string(measure_raw(), tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == MEASURE_DIOD){
		float_to_string(measure_diode(), tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == MEASURE_TEMP_RTD){
		selected_temperature_sensor_type = RTD;
		float_to_string(measure_temperature(), tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == MEASURE_TEMP_NTC){
		selected_temperature_sensor_type = NTC;
		float_to_string(measure_temperature(), tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == CHECK_MEASURE_VOLT_RANGE){
		range_to_string(voltage_range_requested, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == CHECK_MEASURE_CURR_RANGE){
		range_to_string(current_range_requested, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_MEASURE_VOLT_RANGE){
		voltage_range_requested = tmp_parsed_command.range;
		if(voltage_range_requested != RANGE_AUTO) voltage_range_actual = voltage_range_requested;
		display_need_to_be_rewrited = 1;
		tmp_message_to_send = "OK";

	}else if(tmp_parsed_command.command == SET_MEASURE_CURR_RANGE){
		current_range_requested = tmp_parsed_command.range;
		if(current_range_requested != RANGE_AUTO) current_range_actual = current_range_requested;
		else current_range_actual = RANGE_1; //because auto range in current measurement mode cannot be in range_3 !!!
		display_need_to_be_rewrited = 1;
		tmp_message_to_send = "OK";

	}else if(tmp_parsed_command.command == CHECK_CAL_VREF){
		float_to_string(calibration_constants.vref, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_VREF){
		calibration_constants.vref = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_VOLT_U4_SLOPE){
		float_to_string(calibration_constants.voltage.u4.slope, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_VOLT_U4_SLOPE){
		calibration_constants.voltage.u4.slope = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_VOLT_U4_OFFSET){
		float_to_string(calibration_constants.voltage.u4.offset, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_VOLT_U4_OFFSET){
		calibration_constants.voltage.u4.offset = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_VOLT_U40_SLOPE){
		float_to_string(calibration_constants.voltage.u40.slope, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_VOLT_U40_SLOPE){
		calibration_constants.voltage.u40.slope = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_VOLT_U40_OFFSET){
		float_to_string(calibration_constants.voltage.u40.offset, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_VOLT_U40_OFFSET){
		calibration_constants.voltage.u40.offset = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_VOLT_U400_SLOPE){
		float_to_string(calibration_constants.voltage.u400.slope, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_VOLT_U400_SLOPE){
		calibration_constants.voltage.u400.slope = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_VOLT_U400_OFFSET){
		float_to_string(calibration_constants.voltage.u400.offset, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_VOLT_U400_OFFSET){
		calibration_constants.voltage.u400.offset = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_CURR_I5_SLOPE){
		float_to_string(calibration_constants.current.i5.slope, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_CURR_I5_SLOPE){
		calibration_constants.current.i5.slope = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_CURR_I5_OFFSET){
		float_to_string(calibration_constants.current.i5.offset, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_CURR_I5_OFFSET){
		calibration_constants.current.i5.offset = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_CURR_I40_SLOPE){
		float_to_string(calibration_constants.current.i40.slope, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_CURR_I40_SLOPE){
		calibration_constants.current.i40.slope = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_CURR_I40_OFFSET){
		float_to_string(calibration_constants.current.i40.offset, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_CURR_I40_OFFSET){
		calibration_constants.current.i40.offset = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_CURR_I400_SLOPE){
		float_to_string(calibration_constants.current.i400.slope, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_CURR_I400_SLOPE){
		calibration_constants.current.i400.slope = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_CURR_I400_OFFSET){
		float_to_string(calibration_constants.current.i400.offset, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_CURR_I400_OFFSET){
		calibration_constants.current.i400.offset = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_REFERENCE_R1){
		float_to_string(calibration_constants.reference_r1, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_REFERENCE_R1){
		calibration_constants.reference_r1 = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_REFERENCE_R2){
		float_to_string(calibration_constants.reference_r2, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_REFERENCE_R2){
		calibration_constants.reference_r2 = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_TEMP_RTD_R0){
		float_to_string(calibration_constants.temperature.rtd_r0, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_TEMP_RTD_R0){
		calibration_constants.temperature.rtd_r0 = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_TEMP_NTC_R25){
		float_to_string(calibration_constants.temperature.ntc_r25, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_TEMP_NTC_R25){
		calibration_constants.temperature.ntc_r25 = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_TEMP_RTD_COEFF_A){
		float_to_string(calibration_constants.temperature.rtd_coeff_a, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_TEMP_RTD_COEFF_A){
		calibration_constants.temperature.rtd_coeff_a = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}else if(tmp_parsed_command.command == CHECK_CAL_TEMP_NTC_COEFF_B){
		float_to_string(calibration_constants.temperature.ntc_coeff_b, tmp_buffer);
		tmp_message_to_send = tmp_buffer;

	}else if(tmp_parsed_command.command == SET_CAL_TEMP_NTC_COEFF_B){
		calibration_constants.temperature.ntc_coeff_b = tmp_parsed_command.argument;
		tmp_message_to_send = "OK";
		save_constants_to_eeprom();

	}


	if(*tmp_message_to_send != 0x00){
		send_message_to_destination(tmp_message_to_send, tmp_destination);
	}

}

//parse SCPI command
PARSER_OUTPUT_T scpi_command_parser(char *tmp_string){

	//set up default values
	PARSER_OUTPUT_T tmp_output = {.command = UNKNOWN_COMMAND, .argument = 0.0, .range = RANGE_AUTO};

	//if it isn't a valid SCPI command (don't start with '*' or ':')
	//return with unknown command error
	if(*tmp_string != '*' && *tmp_string != ':') return tmp_output;

	if(does_string_start_with(tmp_string, "*IDN?")){
		tmp_output.command = CHECK_IDN;	return tmp_output;
	}

	//remove first character
	tmp_string++;

	if(does_string_start_with(tmp_string, "MEAS")){
		tmp_string = remove_string_from_string(tmp_string, "MEAS");
		tmp_string = remove_to_separator(tmp_string, ':');

		char tmp_separators[] = {':', '?'};
		uint8_t tmp_separators_count = sizeof(tmp_separators)/sizeof(char);

		if(does_string_start_with(tmp_string, "RES")){
			tmp_string = remove_string_from_string(tmp_string, "RES");
			tmp_string = remove_before_separator(tmp_string, '?');

			if(*tmp_string == '?'){
				tmp_output.command = MEASURE_RES; return tmp_output;

			}else{
				return tmp_output;
			}

		}else if(does_string_start_with(tmp_string, "VOLT")){
			tmp_string = remove_string_from_string(tmp_string, "VOLT");			
			tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);

			if(*tmp_string == '?'){
				tmp_output.command = MEASURE_VOLT; return tmp_output;

			}else if(*tmp_string == ':'){
				tmp_string = remove_string_from_string(tmp_string, ":RANGE");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_MEASURE_VOLT_RANGE; return tmp_output;

				}else if(*tmp_string == ' '){
					tmp_output.command = SET_MEASURE_VOLT_RANGE;
					tmp_output.range = string_to_range(++tmp_string);
					return tmp_output;

				}else{
					return tmp_output;
				}							

			}else{
				return tmp_output;
			}

		}else if(does_string_start_with(tmp_string, "CURR")){
			tmp_string = remove_string_from_string(tmp_string, "CURR");			
			tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);

			if(*tmp_string == '?'){
				tmp_output.command = MEASURE_CURR; return tmp_output;

			}else if(*tmp_string == ':'){
				tmp_string = remove_string_from_string(tmp_string, ":RANGE");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_MEASURE_CURR_RANGE; return tmp_output;

				}else if(*tmp_string == ' '){
					tmp_output.command = SET_MEASURE_CURR_RANGE;
					tmp_output.range = string_to_range(++tmp_string);
					return tmp_output;
					
				}else{
					return tmp_output;
				}
			}

		}else if(does_string_start_with(tmp_string, "RAW")){
			tmp_string = remove_string_from_string(tmp_string, "RAW");
			tmp_string = remove_before_separator(tmp_string, '?');

			if(*tmp_string == '?'){
				tmp_output.command = MEASURE_RAW; return tmp_output;

			}else{
				return tmp_output;
			}

		}else if(does_string_start_with(tmp_string, "DIOD")){
			tmp_string = remove_string_from_string(tmp_string, "DIOD");
			tmp_string = remove_before_separator(tmp_string, '?');

			if(*tmp_string == '?'){
				tmp_output.command = MEASURE_DIOD; return tmp_output;
				
			}else{
				return tmp_output;
			}		

		}else if(does_string_start_with(tmp_string, "TEMP")){
			tmp_string = remove_string_from_string(tmp_string, "TEMP");			
			tmp_string = remove_string_before_separators(tmp_string, tmp_separators, tmp_separators_count);

			if(*tmp_string == '?'){
				//measure temperature with last selected type of sensor
				if(selected_temperature_sensor_type == RTD) tmp_output.command = MEASURE_TEMP_RTD;
				else tmp_output.command = MEASURE_TEMP_NTC;

				return tmp_output;

			}else if(*tmp_string == ':'){
				tmp_string = remove_string_from_string(tmp_string, ":");

				if(does_string_start_with(tmp_string, "RTD?")) tmp_output.command = MEASURE_TEMP_RTD;
				else if(does_string_start_with(tmp_string, "NTC?")) tmp_output.command = MEASURE_TEMP_NTC;

				return tmp_output;

			}

		}else{
			return tmp_output;
		}

	}else if(does_string_start_with(tmp_string, "CAL")){
		tmp_string = remove_string_from_string(tmp_string, "CAL");
		tmp_string = remove_to_separator(tmp_string, ':');

		if(does_string_start_with(tmp_string, "VREF")){
			tmp_string = remove_string_from_string(tmp_string, "VREF");

			if(*tmp_string  == '?'){
				tmp_output.command = CHECK_CAL_VREF; return tmp_output;
			}else if(*tmp_string == ' '){
				tmp_output.command = SET_CAL_VREF;
				tmp_output.argument = string_to_float(++tmp_string);
				return tmp_output;
			}else{
				return tmp_output;
			}

		}else if(does_string_start_with(tmp_string, "SLOPE")){
			tmp_string = remove_string_from_string(tmp_string, "SLOPE");
			tmp_string = remove_to_separator(tmp_string, ':');

			if(does_string_start_with(tmp_string, "V4DC")){
				tmp_string = remove_string_from_string(tmp_string, "V4DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_VOLT_U4_SLOPE; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_VOLT_U4_SLOPE;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "V40DC")){
				tmp_string = remove_string_from_string(tmp_string, "V40DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_VOLT_U40_SLOPE; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_VOLT_U40_SLOPE;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "V400DC")){
				tmp_string = remove_string_from_string(tmp_string, "V400DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_VOLT_U400_SLOPE; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_VOLT_U400_SLOPE;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "A5DC")){
				tmp_string = remove_string_from_string(tmp_string, "A5DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_CURR_I5_SLOPE; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_CURR_I5_SLOPE;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "MA40DC")){
				tmp_string = remove_string_from_string(tmp_string, "MA40DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_CURR_I40_SLOPE; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_CURR_I40_SLOPE;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "MA400DC")){
				tmp_string = remove_string_from_string(tmp_string, "MA400DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_CURR_I400_SLOPE; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_CURR_I400_SLOPE;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else{
				return tmp_output;
			}


		}else if(does_string_start_with(tmp_string, "OFFSET")){
			tmp_string = remove_string_from_string(tmp_string, "OFFSET");
			tmp_string = remove_to_separator(tmp_string, ':');

			if(does_string_start_with(tmp_string, "V4DC")){
				tmp_string = remove_string_from_string(tmp_string, "V4DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_VOLT_U4_OFFSET; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_VOLT_U4_OFFSET;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "V40DC")){
				tmp_string = remove_string_from_string(tmp_string, "V40DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_VOLT_U40_OFFSET; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_VOLT_U40_OFFSET;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "V400DC")){
				tmp_string = remove_string_from_string(tmp_string, "V400DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_VOLT_U400_OFFSET; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_VOLT_U400_OFFSET;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "A5DC")){
				tmp_string = remove_string_from_string(tmp_string, "A5DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_CURR_I5_OFFSET; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_CURR_I5_OFFSET;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "MA40DC")){
				tmp_string = remove_string_from_string(tmp_string, "MA40DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_CURR_I40_OFFSET; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_CURR_I40_OFFSET;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "MA400DC")){
				tmp_string = remove_string_from_string(tmp_string, "MA400DC");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_CURR_I400_OFFSET; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_CURR_I400_OFFSET;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else{
				return tmp_output;
			}			

		}else if(does_string_start_with(tmp_string, "R1")){
			tmp_string = remove_string_from_string(tmp_string, "R1");

			if(*tmp_string  == '?'){
				tmp_output.command = CHECK_CAL_REFERENCE_R1; return tmp_output;
			}else if(*tmp_string == ' '){
				tmp_output.command = SET_CAL_REFERENCE_R1;
				tmp_output.argument = string_to_float(++tmp_string);
				return tmp_output;
			}else{
				return tmp_output;
			}

		}else if(does_string_start_with(tmp_string, "R2")){
			tmp_string = remove_string_from_string(tmp_string, "R2");

			if(*tmp_string  == '?'){
				tmp_output.command = CHECK_CAL_REFERENCE_R2; return tmp_output;
			}else if(*tmp_string == ' '){
				tmp_output.command = SET_CAL_REFERENCE_R2;
				tmp_output.argument = string_to_float(++tmp_string);
				return tmp_output;
			}else{
				return tmp_output;
			}

		}else if(does_string_start_with(tmp_string, "TEMP")){
			tmp_string = remove_string_from_string(tmp_string, "TEMP");
			tmp_string = remove_to_separator(tmp_string, ':');

			if(does_string_start_with(tmp_string, "RTD_R0")){
				tmp_string = remove_string_from_string(tmp_string, "RTD_R0");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_TEMP_RTD_R0; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_TEMP_RTD_R0;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "NTC_R25")){
				tmp_string = remove_string_from_string(tmp_string, "NTC_R25");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_TEMP_NTC_R25; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_TEMP_NTC_R25;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "RTD_COEFF_A")){
				tmp_string = remove_string_from_string(tmp_string, "RTD_COEFF_A");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_TEMP_RTD_COEFF_A; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_TEMP_RTD_COEFF_A;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else if(does_string_start_with(tmp_string, "NTC_COEFF_B")){
				tmp_string = remove_string_from_string(tmp_string, "NTC_COEFF_B");

				if(*tmp_string  == '?'){
					tmp_output.command = CHECK_CAL_TEMP_NTC_COEFF_B; return tmp_output;
				}else if(*tmp_string == ' '){
					tmp_output.command = SET_CAL_TEMP_NTC_COEFF_B;
					tmp_output.argument = string_to_float(++tmp_string);
					return tmp_output;
				}else{
					return tmp_output;
				}

			}else{
				return tmp_output;
			}


		}else{
			return tmp_output;		
		}

	}

	return tmp_output;		
}

//called when data from TCP arrived
void esp_command_arrived(TCP_MESSAGE_T tmp_message){
	
	erase_display();

	MESSAGE_SOURCE_T tmp_source_of_message = {.from = ESP, .channel = tmp_message.channel};

	PARSER_OUTPUT_T tmp_parser_output;
	tmp_parser_output = scpi_command_parser(tmp_message.message);

	scpi_command_executor(tmp_parser_output, tmp_source_of_message);
	
}

//called when data from USB arrived
void usb_command_arrived(char *tmp_string){
	
	erase_display();

	MESSAGE_SOURCE_T tmp_source_of_message = {.from = USB, .channel = 0};

	PARSER_OUTPUT_T tmp_parser_output;
	tmp_parser_output = scpi_command_parser(tmp_string);
	
	scpi_command_executor(tmp_parser_output, tmp_source_of_message);

}

//called when something (string) arrived on uart
void string_arrived(char *tmp_string){
	
	//in case of empty string
	if(*tmp_string == 0) return;

	if(does_string_start_with(tmp_string, "WIFI CONNECTED")){
		esp_state.connected_to_wifi = 1;
	
	}else if(does_string_start_with(tmp_string, "WIFI DISCONNECT")){
		esp_state.connected_to_wifi = 0;
		esp_state.wifi_got_ip = 0;

	}else if(does_string_start_with(tmp_string, "WIFI GOT IP")){
		esp_state.wifi_got_ip = 1;

	}else if(does_string_start_with(tmp_string, "+CIFSR:STAIP,")){
		tmp_string += strlen("+CIFSR:STAIP,");
		parse_ip_address(tmp_string, &esp_state.ip_address);
	
	}else if(does_string_start_with(tmp_string, "+CIFSR:STAMAC,")){

	}else if(does_string_start_with(tmp_string, "OK")){
		esp_state.last_command_executed_successfuly = EXECUTED_OK;

	}else if(does_string_start_with(tmp_string, "ERROR")){
		esp_state.last_command_executed_successfuly = EXECUTED_ERROR;

	}else if(does_string_start_with(tmp_string, "FAIL")){
		esp_state.last_command_executed_successfuly = EXECUTED_FAIL;

	//when data from TCP arrived
	}else if(does_string_start_with(tmp_string, "+IPD,")){

		//message example "+IPD,0,5:lol" - format +IPD,Channel,LenghtOfMessageIncludingCRLF:Message

		TCP_MESSAGE_T tmp_message;
		tmp_message.channel = 0;

		//remove starting from string
		tmp_string = remove_string_from_string(tmp_string, "+IPD,");
		
		//find channel number
		while(*tmp_string != ',' && *tmp_string != 0x00){
			tmp_message.channel *= 10;
			tmp_message.channel += char_to_digit(*tmp_string);
			tmp_string++;
		}

		if(*tmp_string != 0x00){tmp_string++;}

		//remove everything before ':' (including)
		tmp_string = remove_to_separator(tmp_string, ':');

		//save parsed message from TCP
		tmp_message.message = tmp_string;

		esp_command_arrived(tmp_message);

	//when first char is like SCPI command
	//check if it is SCPI command sent to uart
	}else if(*tmp_string == '*' || *tmp_string == ':'){

		usb_command_arrived(tmp_string);

	}

}