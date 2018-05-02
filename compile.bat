REM #/* +--------------------------------------------+ */
REM #/* |              Zenit Multimeter              | */
REM #/* |                compile.bat                 | */
REM #/* | (c)copyright nitram147 [Martin Ersek] 2018 | */
REM #/* +--------------------------------------------+ */

echo compiling ...
avr-gcc -mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL -c main.c -o main.o
avr-gcc -mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL -c lcd.c -o lcd.o
avr-gcc -mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL -c uart.c -o uart.o
avr-gcc -mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL -c esp.c -o esp.o
avr-gcc -mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL -Wall -Werror -Wextra -c parser.c -o parser.o
avr-gcc -mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL -c calibration.c -o calibration.o
avr-gcc -mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL -c spi.c -o spi.o
avr-gcc -mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL -c buttons_switches.c -o buttons_switches.o
avr-gcc -mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL -c measure.c -o measure.o
avr-gcc -mmcu=atmega328p -std=c99 -Os -DF_CPU=16000000UL -c display.c -o display.o

avr-gcc -g -mmcu=atmega328p -o main.elf main.o lcd.o uart.o esp.o parser.o calibration.o spi.o buttons_switches.o measure.o display.o -lm
avr-objdump -h -S main.elf > main.lst
avr-objcopy -j .text -j .data -O ihex main.elf main.hex
avrdude -c arduino -p atmega328p -P COM1 -b 57600 -U flash:w:main.hex
