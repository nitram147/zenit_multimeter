/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |               calibration.h                | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#ifndef CALIBRATION_H
#define CALIBRATION_H

//just some random number to mark if we have already saved constants to eeprom
#define EEPROM_USED 47
//default value of specific eeprom address
#define EEPROM_UNUSED 0xff
//eeprom addresses
#define EEPROM_STATE_ADDRESS 10
//starting address of calibration_constants struct object
#define EEPROM_CONSTANTS_ADDRESS 20

typedef struct {
	float slope;
	float offset;
} linear_correction_t;

typedef struct {
	linear_correction_t u4;
	linear_correction_t u40;
	linear_correction_t u400;
} voltage_constants_t;

typedef struct{
	linear_correction_t i5;
	linear_correction_t i40;
	linear_correction_t i400;
} current_constants_t;

typedef struct{
	float rtd_r0;
	float rtd_coeff_a;
	float ntc_r25;
	float ntc_coeff_b;
} temperature_constants_t;

typedef struct {
	float vref;
	voltage_constants_t voltage;
	current_constants_t current;
	float reference_r1;
	float reference_r2;
	temperature_constants_t temperature;
} calibration_t;

calibration_t calibration_constants;

uint8_t are_constants_already_saved_in_eeprom();

void save_constants_to_eeprom();

void retrieve_constants_from_eeprom();

void init_calibration_constants();

#endif