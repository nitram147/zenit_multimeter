/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                 measure.h                  | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#ifndef MEASURE_H
#define MEASURE_H

typedef struct{
	float adc_raw;
	float voltage;
	float current;
	float resistance;
	float diode;
	float temperature;
	uint8_t overflow;
} MEASURED_VALUES_T;

typedef struct{
	long code;
	uint8_t sign;
	uint8_t overflow;
	uint8_t underflow;
} ADC_READING_T;

MEASURED_VALUES_T current_measured_values;

ADC_READING_T read_adc();

float measure_resistance();

float measure_voltage();

float measure_current();

float measure_raw();

float measure_diode();

float measure_temperature();

#endif