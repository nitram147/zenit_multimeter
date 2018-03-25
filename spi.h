/* +--------------------------------------------+ */
/* |              Zenit Multimeter              | */
/* |                   spi.h                    | */
/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
/* +--------------------------------------------+ */
#ifndef SPI_H
#define SPI_H

#define SCK_DDR DDRB
#define SCK_PORT PORTB
#define SCK_BIT PB5

#define MISO_DDR DDRB
#define MISO_PORT PORTB
#define MISO_BIT PB4

#define MOSI_DDR DDRB
#define MOSI_PORT PORTB
#define MOSI_BIT PB3

#define CSADC_DDR DDRB
#define CSADC_PORT PORTB
#define CSADC_BIT PB2

#define CSSHIFT_DDR DDRB
#define CSSHIFT_PORT PORTB
#define CSSHIFT_BIT PB1

#define LOAD_DDR DDRB
#define LOAD_PORT PORTB
#define LOAD_BIT PB0

void spi_adc_select();

void spi_adc_deselect();

void spi_load_pulse();

void spi_shift_pulse();

void spi_mode0();

void spi_mode1();

void spi_init();

uint8_t spi_transfer(uint8_t tmp_byte);

#endif