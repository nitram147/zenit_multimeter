/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                 measure.c                  | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#include <avr/io.h>
#include <util/delay.h>

#include "measure.h"
#include "spi.h"
#include "buttons_switches.h"
#include "calibration.h"
#include "esp.h" //required for correct inclusion of parser.h
#include "parser.h" //measuring ranges definition and global variables (measurement ranges)
#include "display.h"

MEASURED_VALUES_T current_measured_values = {0,0,0,0,0,0,0};

ADC_READING_T read_adc(){

	uint8_t byte_0, byte_1, byte_2, byte_3;
  	ADC_READING_T current_adc_reading = {0,0,0,0};

  	spi_mode1();
	spi_adc_select();

	byte_0 = spi_transfer(0); //read MSB
	byte_1 = spi_transfer(0); //
	byte_2 = spi_transfer(0); //
	byte_3 = spi_transfer(0); //read LSB

	spi_adc_deselect();

	current_adc_reading.sign = (byte_0 & 0b00100000) >> 5; //sign bit 1=pos, 0=neg

	/*	Bit 28 (fourth output bit) is the most significant bit (MSB) of the result.
    	This bit in conjunction with Bit 29 also provides the underrange or overrange indication.
    	If both Bit 29 and Bit 28 are HIGH, the differential input voltage is above +FS.
    	If both Bit 29 and Bit 28 are LOW, the differential input voltage is below –FS. */

    if((byte_0 & 0b00110000) == 0b00110000) current_adc_reading.overflow = 1;

    if((byte_0 & 0b00110000) == 0b00000000) current_adc_reading.underflow = 1;

	byte_3 = byte_3 & 0b11100000; //clear the 4 LSBs
	byte_0 = byte_0 & 0b00011111; //clear the 4 MSBs

	current_adc_reading.code =  ((long) byte_3 >> 5);
	current_adc_reading.code += ((long) byte_2 << 3);
	current_adc_reading.code += ((long) byte_1 << 11);
	current_adc_reading.code += ((long) byte_0 << 19);

	if(current_adc_reading.sign){
		current_measured_values.adc_raw = (float)current_adc_reading.code; //positive
	}else{
		current_measured_values.adc_raw = (float)current_adc_reading.code - 16777216.0; //negative
		current_adc_reading.code -= 16777216;
	}

	current_measured_values.overflow = current_adc_reading.overflow;
	return current_adc_reading;
}

float measure_resistance(){

	ADC_READING_T tmp_ref_reading, tmp_meas_reading;

	set_switches(FE_RES_REF); //set switches for R reference measurement
	read_adc(); _delay_ms(190); //discard first sample and trigger new measurement
	tmp_ref_reading = read_adc(); //valid reference reading

	set_switches(FE_RES_MEAS);  //set switches for R reference measurement
	read_adc(); _delay_ms(190); //discard first sample and trigger new measurement
	tmp_meas_reading = read_adc(); //valid measurement reading
	
	float rmeas = -calibration_constants.reference_r1 * calibration_constants.reference_r2;
	rmeas /= (calibration_constants.reference_r1 - calibration_constants.reference_r2 * (float)tmp_ref_reading.code / (float)tmp_meas_reading.code);
	
	current_measured_values.resistance = rmeas;

	//switch to R_MEASUREMENT mode
	if(display_interface_mode != R_MEASUREMENT) display_interface_mode = R_MEASUREMENT;
	display_need_to_be_rewrited = 1;

	return rmeas;
}

float measure_voltage(){

	ADC_READING_T tmp_adc_reading = {0,0,0,0};
	float tmp_measured_voltage = 0.0;
	uint8_t tmp_range_changed = 0; //flag set to 1 when range has change (auto range mode)

	//if requested mode is fixed (RANGE_1-3) set actual range to requested range
	if(voltage_range_requested != RANGE_AUTO && voltage_range_requested != UNKNOWN_RANGE){
		voltage_range_actual = voltage_range_requested;
	}


	//set switches according to selected measurement range
	if(voltage_range_actual == RANGE_1){
		//set switches for voltage measurement, range 4V
		set_switches(FE_UDC | FE_UDIV2 | FE_GAIN2);

	}else if(voltage_range_actual == RANGE_2){
		//set switches for voltage measurement, range 40V
		set_switches(FE_UDC | FE_UDIV20 | FE_GAIN2);

	}else if(voltage_range_actual == RANGE_3){
		//set switches for voltage measurement, range 400V
		set_switches(FE_UDC | FE_UDIV200 | FE_GAIN2);

	}

	//read ADC, first sample after switching the range will not be valid !
	tmp_adc_reading = read_adc();


	if(voltage_range_actual == RANGE_1 && !tmp_range_changed){

		tmp_measured_voltage = (float)tmp_adc_reading.code;
		tmp_measured_voltage *= calibration_constants.vref * calibration_constants.voltage.u4.slope;
		tmp_measured_voltage += calibration_constants.voltage.u4.offset;

		if(voltage_range_requested == RANGE_AUTO 
			&& (tmp_measured_voltage > 4.0 || tmp_measured_voltage < -4.0 
			|| tmp_adc_reading.overflow || tmp_adc_reading.underflow)){

			voltage_range_actual = RANGE_2;
			tmp_range_changed = 1;
			
			set_switches(FE_UDC | FE_UDIV20 | FE_GAIN2); //set switches for voltage measurement, range 40V
			read_adc(); _delay_ms(190); //re-trigger ADC conversion after range change
		}
	}


	if(voltage_range_actual == RANGE_2 && !tmp_range_changed){

		tmp_measured_voltage = (float)tmp_adc_reading.code;
		tmp_measured_voltage *= calibration_constants.vref * calibration_constants.voltage.u40.slope;
		tmp_measured_voltage += calibration_constants.voltage.u40.offset;

		if(voltage_range_requested == RANGE_AUTO 
			&& (tmp_measured_voltage > 40.0 || tmp_measured_voltage < -40.0 
			|| tmp_adc_reading.overflow || tmp_adc_reading.underflow)){

			voltage_range_actual = RANGE_3;
			tmp_range_changed = 1;
			
			set_switches(FE_UDC | FE_UDIV200 | FE_GAIN2); //set switches for voltage measurement, range 400V
			read_adc(); _delay_ms(190); //re-trigger ADC conversion after range change

		}else if(voltage_range_requested == RANGE_AUTO 
			&& (tmp_measured_voltage <= 4.0 && tmp_measured_voltage >= -4.0)){

			voltage_range_actual = RANGE_1;
			tmp_range_changed = 1;

			set_switches(FE_UDC | FE_UDIV2 | FE_GAIN2); //set switches for voltage measurement, range 4V
			read_adc(); _delay_ms(190); //re-trigger ADC conversion after range change


		}
	}


	if(voltage_range_actual == RANGE_3 && !tmp_range_changed){

		tmp_measured_voltage = (float)tmp_adc_reading.code;
		tmp_measured_voltage *= calibration_constants.vref * calibration_constants.voltage.u400.slope;
		tmp_measured_voltage += calibration_constants.voltage.u400.offset;

		if(voltage_range_requested == RANGE_AUTO 
			&& (tmp_measured_voltage <= 40.0 && tmp_measured_voltage >= -40.0)){

			voltage_range_actual = RANGE_2;
			tmp_range_changed = 1;

			set_switches(FE_UDC | FE_UDIV20 | FE_GAIN2); //set switches for voltage measurement, range 4V
			read_adc(); _delay_ms(190); //re-trigger ADC conversion after range change


		}
	}

	current_measured_values.voltage = tmp_measured_voltage;

	//switch to R_MEASUREMENT mode
	if(display_interface_mode != U_MEASUREMENT) display_interface_mode = U_MEASUREMENT;
	display_need_to_be_rewrited = 1;

	return tmp_measured_voltage;
}

float measure_current(){

	ADC_READING_T tmp_adc_reading = {0,0,0,0};
	float tmp_measured_current = 0.0;

	// const float two_to_25 = 33554432.0; // 2^25

	// const float Rshunt40mA = 1.0 * 46; //shunt resistance for 40mA range (Ohm) + gain of 46
	// const float Rshunt400mA = 1.0 * 2; //shunt resistance for 400mA range (Ohm) + gain of 2
	// const float Rshunt5A = 0.01 * 46; //shunt resistance for 5A range (Ohm) + gain of 46

	//if requested mode is fixed (RANGE_1-3) set actual range to requested range
	if(current_range_requested != RANGE_AUTO && current_range_requested != UNKNOWN_RANGE){
		current_range_actual = current_range_requested;
	}


	//set switches according to selected measurement range
	if(current_range_actual == RANGE_1){
		//set switches for current measurement, range 40mA
		set_switches(FE_I40MA | FE_GAIN46);

	}else if(current_range_actual == RANGE_2){
		//set switches for current measurement, range 400mA
		set_switches(FE_I400MA | FE_GAIN2);

	}else if(current_range_actual == RANGE_3){
		//set switches for current measurement, range 5A
		set_switches(FE_I5A | FE_GAIN46);

	}

	//read ADC, first sample after switching the range will not be valid !
	tmp_adc_reading = read_adc();


	if(current_range_actual == RANGE_1){

		tmp_measured_current = (float)tmp_adc_reading.code;
		tmp_measured_current *= calibration_constants.vref;
		tmp_measured_current *= calibration_constants.current.i40.slope;
		tmp_measured_current += calibration_constants.current.i40.offset;


		if(current_range_requested == RANGE_AUTO 
			&& (tmp_measured_current > 0.040 || tmp_adc_reading.overflow || tmp_adc_reading.underflow)){

			current_range_actual = RANGE_2;
			
			set_switches(FE_I400MA | FE_GAIN2); //set switches for current measurement, range 400mA
			read_adc(); _delay_ms(190); //re-trigger ADC conversion after range change
		}
	}


	if(current_range_actual == RANGE_2){

		tmp_measured_current = (float)tmp_adc_reading.code;
		tmp_measured_current *= calibration_constants.vref;
		tmp_measured_current *= calibration_constants.current.i400.slope;
		tmp_measured_current += calibration_constants.current.i400.offset;


		if(current_range_requested == RANGE_AUTO && tmp_measured_current <= 0.040){

			current_range_actual = RANGE_1;
			
			set_switches(FE_I40MA | FE_GAIN46); //set switches for current measurement, range 40mA
			read_adc(); _delay_ms(190); //re-trigger ADC conversion after range change
		}
	}

	if(current_range_actual == RANGE_3){

		tmp_measured_current = (float)tmp_adc_reading.code;
		tmp_measured_current *= calibration_constants.vref;
		tmp_measured_current *= calibration_constants.current.i5.slope;
		tmp_measured_current += calibration_constants.current.i5.offset;	

	}

	current_measured_values.current = tmp_measured_current;

	//switch to R_MEASUREMENT mode
	if(display_interface_mode != I_MEASUREMENT) display_interface_mode = I_MEASUREMENT;
	display_need_to_be_rewrited = 1;

	return tmp_measured_current;
}

float measure_raw(){
	return current_measured_values.adc_raw;
}

float measure_diode(){
	
	ADC_READING_T tmp_meas_reading;
	float tmp_diode_voltage;

	const float two_to_24 = 16777216.0; // 2^25

	set_switches(FE_RES_MEAS);  //set switches for R reference measurement
	read_adc(); _delay_ms(190); //discard first sample and trigger new measurement
	tmp_meas_reading = read_adc(); //valid measurement reading

	tmp_diode_voltage = (float)tmp_meas_reading.code;
	tmp_diode_voltage /= two_to_24;
	tmp_diode_voltage *= calibration_constants.vref;

	current_measured_values.diode = tmp_diode_voltage;
	
	//switch to D_MEASUREMENT mode
	if(display_interface_mode != D_MEASUREMENT) display_interface_mode = D_MEASUREMENT;
	display_need_to_be_rewrited = 1;

	return tmp_diode_voltage;
}

float measure_temperature(){

	float tmp_temperature = 0.0;

	if(selected_temperature_sensor_type == RTD){

		tmp_temperature = (measure_resistance() - calibration_constants.temperature.rtd_r0);
		tmp_temperature /= (calibration_constants.temperature.rtd_coeff_a * calibration_constants.temperature.rtd_r0);

	}else{

		const float inv_t25 = 1/298.15;

		tmp_temperature = log(measure_resistance() / calibration_constants.temperature.ntc_r25);
		tmp_temperature /= calibration_constants.temperature.ntc_coeff_b;
		tmp_temperature += inv_t25;
		tmp_temperature = 1 / tmp_temperature;

	}

	current_measured_values.temperature = tmp_temperature;

	if(display_interface_mode != T_MEASUREMENT) display_interface_mode = T_MEASUREMENT;
	display_need_to_be_rewrited = 1;

	return tmp_temperature;
}