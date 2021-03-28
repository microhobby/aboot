
#ifndef __IMPLEMENTS_HEADER__
#define __IMPLEMENTS_HEADER__

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

char gethexnib(void) {
	char a;
	a = getch(); putch(a);
	if(a >= 'a') {
		return (a - 'a' + 0x0a);
	} else if(a >= '0') {
		return(a - '0');
	}
	return a;
}


char gethex(void) {
	return (gethexnib() << 4) + gethexnib();
}


void puthex(char ch) {
	char ah;

	ah = ch >> 4;
	if(ah >= 0x0a) {
		ah = ah - 0x0a + 'a';
	} else {
		ah += '0';
	}
	
	ch &= 0x0f;
	if(ch >= 0x0a) {
		ch = ch - 0x0a + 'a';
	} else {
		ch += '0';
	}
	
	putch(ah);
	putch(ch);
}


void putch(char ch)
{
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = ch;
}


char getch(void)
{
	uint32_t count = 0;
	while(!(UCSR0A & _BV(RXC0))){
		/* 20060803 DojoCorp:: Addon coming from the previous Bootloader*/               
		/* HACKME:: here is a good place to count times*/
		count++;
		if (count > MAX_TIME_COUNT)
			app_start();
	}

	return UDR0;
}


void getNch(uint8_t count)
{
	while(count--) {
		getch();		
	}
}


void byte_response(uint8_t val)
{
	if (getch() == ' ') {
		putch(0x14);
		putch(val);
		putch(0x10);
	} else {
		if (++error_count == MAX_ERROR_COUNT)
			app_start();
	}
}


void nothing_response(void)
{
	if (getch() == ' ') {
		putch(0x14);
		putch(0x10);
	} else {
		if (++error_count == MAX_ERROR_COUNT)
			app_start();
	}
}

void flash_led(uint8_t count)
{
	while (count--) {
		LED_PORT |= _BV(LED);
		_delay_ms(100);
		LED_PORT &= ~_BV(LED);
		_delay_ms(100);
	}
}

void turn_led_on()
{
    LED_PORT |= _BV(LED);
}

int return_key () 
{
	return 42;
}

int return_challenge ()
{
	return 24;
}

void increment_flash_counter()
{
	/* only increment one time */
	if (flash_count_var == 0xFF) {
		/* read last byte - last byte is flash counter */
		flash_count_var = eeprom_read_byte((uint8_t *)512);
		/* increment */
		flash_count_var++;
		eeprom_update_byte((uint8_t *)512, flash_count_var);
	}
}

void flash_counter_to_zero()
{
	uint8_t count = eeprom_read_byte((uint8_t *)512);
	if (count != 0) {
		eeprom_write_byte((uint8_t *)512, 0);
	}
	asm volatile("nop             \n\t");
	asm volatile("nop             \n\t");
	asm volatile("nop             \n\t");
	asm volatile("nop             \n\t");
	asm volatile("nop             \n\t");
	asm volatile("nop             \n\t");
	asm volatile("nop             \n\t");
}

int flash_count()
{
	return eeprom_read_byte((uint8_t *)512);
}

#endif
