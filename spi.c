/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                   spi.c                    | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#include <avr/io.h>
#include <util/delay.h>

#include "spi.h"

//enable ADC chip select
void spi_adc_select(){ CSADC_PORT &=~(1<<CSADC_BIT); }

//disable ADC chip select
void spi_adc_deselect(){ CSADC_PORT |= (1<<CSADC_BIT); }

//generate short pulse on load pin
void spi_load_pulse(){
	LOAD_PORT &=~(1<<LOAD_BIT);
	_delay_us(5);
	LOAD_PORT |= (1<<LOAD_BIT);	
}

//generate short pulse on csshift pin
void spi_shift_pulse(){
	CSSHIFT_PORT &=~(1<<CSSHIFT_BIT);
	_delay_us(5);
	CSSHIFT_PORT |= (1<<CSSHIFT_BIT);	
}

//switch SPI to mode0
void spi_mode0(){
	//clear CPOL and CPHA - "arduino spi mode 0"
	SPCR &= (~(1<<CPOL))&(~(1<<CPHA));
}

//switch SPI to mode1
void spi_mode1(){
	//set CPHA, clear CPOL - "arduino spi mode 1"
	SPCR |= (1<<CPHA);
	SPCR &=~(1<<CPOL);
}

//initialize SPI and switch to mode1
void spi_init(){
	//set pins as output
	MOSI_DDR |= (1<<MOSI_BIT);
	SCK_DDR |= (1<<SCK_BIT);
	CSADC_DDR |= (1<<CSADC_BIT);
	CSSHIFT_DDR |= (1<<CSSHIFT_BIT);
	LOAD_DDR |= (1<<LOAD_BIT);

	//set pin as input
	MISO_DDR &=~(1<<MISO_BIT);

	//set pins high
	MISO_PORT |= (1<<MISO_BIT);
	CSADC_PORT |= (1<<CSADC_BIT);
	CSSHIFT_PORT |= (1<<CSSHIFT_BIT);
	LOAD_PORT |= (1<<LOAD_BIT);

	//spi control register
	//enable spi, master mode, prescaler 128 (125kHz),
	SPCR |= (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);
	//MSB first
	SPCR &=~(1<<DORD);

	spi_mode1();

}

//transfer and receive byte from SPI bus
uint8_t spi_transfer(uint8_t tmp_byte){
	//transfer byte
	SPDR = tmp_byte;
	//wait until transfer completed flag is set
	while(!(SPSR & (1<<SPIF)));
	//return received byte
	return SPDR;
}