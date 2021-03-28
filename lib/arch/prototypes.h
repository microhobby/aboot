
#ifndef __PROTOTYPE_HEADER__
#define __PROTOTYPE_HEADER__

#include <inttypes.h>

/* function prototypes */
void putch(char);
char getch(void);
void getNch(uint8_t);
void byte_response(uint8_t);
void nothing_response(void);
char gethex(void);
void puthex(char);
void flash_led(uint8_t);
void asm_flash_data(void);
void turn_led_on(void);
void increment_flash_counter(void);
void flash_counter_to_zero(void);
int return_key (void);
int return_challenge (void);
int flash_count (void);

/* some variables */
union address_union {
	uint16_t word;
	uint8_t  byte[2];
} address;

union length_union {
	uint16_t word;
	uint8_t  byte[2];
} length;

struct flags_struct {
	unsigned eeprom : 1;
	unsigned rampz  : 1;
} flags;

uint8_t buff[256];
uint8_t address_high;

uint8_t pagesz=0x80;

uint8_t i;
uint8_t bootuart = 0;

uint8_t error_count = 0;
uint8_t flash_count_var = 0xFF;

void (*app_start)(void) = 0x0000;

#endif