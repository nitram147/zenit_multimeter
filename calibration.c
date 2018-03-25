/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |               calibration.c                | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#include <avr/io.h>
#include <avr/eeprom.h>

#include "calibration.h"

//check if our constants are already saved in eeprom
uint8_t are_constants_already_saved_in_eeprom(){
	return (eeprom_read_byte((uint8_t*)EEPROM_STATE_ADDRESS) != EEPROM_UNUSED);
}

//save constants struct to eeprom
void save_constants_to_eeprom(){
	//setp eeprom state address value to eeprom used to indicate that constants were saved into eeprom
	eeprom_write_byte((uint8_t*)EEPROM_STATE_ADDRESS, EEPROM_USED);
	//save constants struct to eeprom constants address
	eeprom_write_block(&calibration_constants, (void*)EEPROM_CONSTANTS_ADDRESS, sizeof(calibration_constants));
}

//read constants struct from eeprom and save it into our global constants struct
void retrieve_constants_from_eeprom(){
	eeprom_read_block(&calibration_constants, (void*)EEPROM_CONSTANTS_ADDRESS, sizeof(calibration_constants));
}

//retrieve calibration constants from eeprom or set default constants values
void init_calibration_constants(){

	if(are_constants_already_saved_in_eeprom()){

		retrieve_constants_from_eeprom();

	}else{

		const float two_to_24 = 16777216.0; // 2^24
		//const float two_to_25 = 33554432.0; // 2^25

		const float rshunt_40mA = 1.175 * 46.4642; // Shunt resistance for 40mA range (Ohm) + gain of 46
		const float rshunt_400ma = 1.175 * 2.0; // Shunt resistance for 400mA range (Ohm) + gain of 2
		const float rshunt_5a = 0.01 * 46.4642; // Shunt resistance for 5A range (Ohm) + gain of 46


		calibration_constants.vref = 5.000;	//reference voltage (V)

		calibration_constants.voltage.u4.slope = (2490.0 + 747.0) / (747.0) * 0.5 / two_to_24; // Division ratio (Uin/Uadc) for 4V range
		calibration_constants.voltage.u4.offset = 0.0;

		calibration_constants.voltage.u40.slope = (2490.0 + 57.4615) / 57.4615 * 0.5 / two_to_24; //Division ratio (Uin/Uadc) for 40V range
		calibration_constants.voltage.u40.offset = 0.0;

		calibration_constants.voltage.u400.slope = (2490.0 + 5.5583) / 5.5583 * 0.5 / two_to_24; // Division ratio (Uin/Uadc) for 400V range
		calibration_constants.voltage.u400.offset = 0.0;

		calibration_constants.current.i5.slope = 1 / (rshunt_5a * two_to_24);
		calibration_constants.current.i5.offset = 0.0;

		calibration_constants.current.i40.slope = 1 / (rshunt_40mA * two_to_24);
		calibration_constants.current.i40.offset = 0.0;

		calibration_constants.current.i400.slope = 1 / (rshunt_400ma * two_to_24);
		calibration_constants.current.i400.offset = 0.0;

		calibration_constants.reference_r1 = 1866.26;	// Reference resistance series (Ohm)
		calibration_constants.reference_r2 = 1866.30;	// Reference resistance parallel (Ohm)

		calibration_constants.temperature.rtd_r0 = 100.0;
		calibration_constants.temperature.rtd_coeff_a = 0.003925;

		calibration_constants.temperature.ntc_r25 = 1000.0;
		calibration_constants.temperature.ntc_coeff_b = 3000.0;

	}
	
}