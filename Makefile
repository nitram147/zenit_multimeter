#/* +--------------------------------------------+ */
#/* |              Zenit Multimeter              | */
#/* |                  Makefile                  | */
#/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
#/* +--------------------------------------------+ */

# Compiler to use
CC=avr-gcc

# Compiler parameters
# MCU atmega328p, C99 standart, Space optimalizer, 16MHz clock rate
CFLAGS=-mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL
FFLAGS=-g -mmcu=atmega328p

# avrdude programmer option
PROGRAMMER=arduino
# serial port of programmer
PORT=/dev/ttyUSB0
# baud rate of arduino bootloader
BAUD_RATE=57600

# compile all required programs
all: main.hex

# compile program into main.hex
main.hex: main.o lcd.o uart.o esp.o parser.o calibration.o spi.o buttons_switches.o measure.o display.o
	$(CC) $(FFLAGS) -o main.elf main.o lcd.o uart.o esp.o parser.o calibration.o spi.o buttons_switches.o measure.o display.o -lm
	avr-objdump -h -S main.elf > main.lst
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex

# general rule for our object files
%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

# upload generated hex into MCU
upload: main.hex
	avrdude -c $(PROGRAMMER) -p atmega328p -P $(PORT) -b $(BAUD_RATE) -U flash:w:main.hex

# delete generated object files
clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.lst
	rm -f *.hex