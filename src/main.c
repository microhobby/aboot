#include <defconfigs.h>
#include <prototypes.h>
#include <asm.h>
#include <implements.h>

/* some includes */
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

/* main program starts here */
int main(void)
{
	uint8_t ch,ch2;
	uint16_t w;

	asm volatile("nop\n\t");

	/* check if flash is programmed already, if not start bootloader anyway */
	if(pgm_read_byte_near(0x0000) == 0xFF) {
		/* check eeprom flash counter byte */
		flash_counter_to_zero();
		/* set LED pin as output and turn on for say its virgin */
		LED_DDR |= _BV(LED);
		turn_led_on();
		_delay_ms(1000);
		//eeprom_write_dword(0, return_key);
	}

	/* initialize UART(s) depending on CPU defined */
#ifdef DOUBLE_SPEED
	UCSR0A = (1<<U2X0); //Double speed mode USART0
	UBRR0L = (uint8_t)(F_CPU/(BAUD_RATE*8L)-1);
	UBRR0H = (F_CPU/(BAUD_RATE*8L)-1) >> 8;
#else
	UBRR0L = (uint8_t)(F_CPU/(BAUD_RATE*16L)-1);
	UBRR0H = (F_CPU/(BAUD_RATE*16L)-1) >> 8;
#endif

	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);

	/* Enable internal pull-up resistor on pin D0 (RX), in order
	to supress line noise that prevents the bootloader from
	timing out (DAM: 20070509) */
	DDRD &= ~_BV(PIND0);
	PORTD |= _BV(PIND0);

	/* set LED pin as output */
	LED_DDR |= _BV(LED);
	flash_led(NUM_LED_FLASHES);

	/* forever loop */
	for (;;) {

		/* get character from UART */
		ch = getch();

		/* A bunch of if...else if... gives smaller code than switch...case ! */

		/* Hello is anyone home ? */ 
		if(ch=='0') {
			nothing_response();
		}

		/* Request programmer ID */
		/* Not using PROGMEM string due to boot block in m128 being beyond 64kB boundry  */
		/* Would need to selectively manipulate RAMPZ, and it's only 9 characters anyway so who cares.  */
		else if(ch=='1') {
			if (getch() == ' ') {
				putch(0x14);
				putch('A');
				putch('V');
				putch('R');
				putch(' ');
				putch('I');
				putch('S');
				putch('P');
				putch(0x10);
			} else {
				if (++error_count == MAX_ERROR_COUNT)
					app_start();
			}
		}

		/* AVR ISP/STK500 board commands  DON'T CARE so default nothing_response */
		else if(ch=='@') {
			ch2 = getch();
			if (ch2>0x85) getch();
			nothing_response();
		}

		/* AVR ISP/STK500 board requests */
		else if(ch=='A') {
			ch2 = getch();
			if(ch2==0x80) byte_response(HW_VER);		// Hardware version
			else if(ch2==0x81) byte_response(SW_MAJOR);	// Software major version
			else if(ch2==0x82) byte_response(SW_MINOR);	// Software minor version
			else if(ch2==0x98) byte_response(0x03);		// Unknown but seems to be required by avr studio 3.56
			else byte_response(0x00);				// Covers various unnecessary responses we don't care about
		}

		/* Device Parameters  DON'T CARE, DEVICE IS FIXED  */
		else if(ch=='B') {
			getNch(20);
			nothing_response();
		}

		/* Parallel programming stuff  DON'T CARE  */
		else if(ch=='E') {
			getNch(5);
			nothing_response();
		}

		/* P: Enter programming mode  */
		/* R: Erase device, don't care as we will erase one page at a time anyway.  */
		else if(ch=='P' || ch=='R') {
			nothing_response();
		}

		/* Leave programming mode  */
		else if(ch=='Q') {
			nothing_response();
#ifdef WATCHDOG_MODS
			// autoreset via watchdog (sneaky!)
			WDTCSR = _BV(WDE);
			while (1); // 16 ms
#endif
		}

		/* Set address, little endian. EEPROM in bytes, FLASH in words  */
		/* Perhaps extra address bytes may be added in future to support > 128kB FLASH.  */
		/* This might explain why little endian was used here, big endian used everywhere else.  */
		else if(ch=='U') {
			address.byte[0] = getch();
			address.byte[1] = getch();
			nothing_response();
		}

		/* Universal SPI programming command, disabled.  Would be used for fuses and lock bits.  */
		else if(ch=='V') {
			if (getch() == 0x30) {
				getch();
				ch = getch();
				getch();
				if (ch == 0) {
					byte_response(SIG1);
				} else if (ch == 1) {
					byte_response(SIG2); 
				} else {
					byte_response(SIG3);
				} 
			} else {
				getNch(3);
				byte_response(0x00);
			}
		}

		/* Write memory, length is big endian and is in bytes  */
		else if(ch=='d') {

			length.byte[1] = getch();
			length.byte[0] = getch();
			flags.eeprom = 0;

			if (getch() == 'E') flags.eeprom = 1;

			for (w=0;w<length.word;w++) {
				buff[w] = getch(); // Store data in buffer, can't keep up with serial data stream whilst programming pages
			}

			if (getch() == ' ') {
				if (flags.eeprom) { //Write to EEPROM one byte at a time
					address.word <<= 1;

					for(w=0;w<length.word;w++) {
						while(EECR & (1<<EEPE));
						EEAR = (uint16_t)(void *)address.word;
						EEDR = buff[w];
						EECR |= (1<<EEMPE);
						EECR |= (1<<EEPE);
						address.word++;
					}			
				}
				else
				{
					/* Write to FLASH one page at a time */
					if (address.byte[1]>127) 
						address_high = 0x01;	//Only possible with m128, m256 will need 3rd address byte. FIXME
					else 
						address_high = 0x00;

					address.word = address.word << 1;	        //address * 2 -> byte location
					
					if ((length.byte[0] & 0x01)) 
						length.word++;	//Even up an odd number of bytes
					cli();					//Disable interrupts, just to be sure

#if defined(EEPE)
					while(bit_is_set(EECR,EEPE));			//Wait for previous EEPROM writes to complete
#else
					while(bit_is_set(EECR,EEWE));			//Wait for previous EEPROM writes to complete
#endif

					asm_flash_data();
					increment_flash_counter();
					/* Should really add a wait for RWW section to be enabled, don't actually need it since we never */
					/* exit the bootloader without a power cycle anyhow */
				}
				putch(0x14);
				putch(0x10);
			} else {
				if (++error_count == MAX_ERROR_COUNT)
					app_start();
			}		
		}

		/* Read memory block mode, length is big endian.  */
		else if(ch=='t') {
			length.byte[1] = getch();
			length.byte[0] = getch();

			address.word = address.word << 1;	        // address * 2 -> byte location
			if (getch() == 'E') flags.eeprom = 1;
			else flags.eeprom = 0;
			if (getch() == ' ') {		                // Command terminator
				putch(0x14);
				for (w=0;w < length.word;w++) {		        // Can handle odd and even lengths okay
					if (flags.eeprom) {	                        // Byte access EEPROM read
						while(EECR & (1<<EEPE));
						EEAR = (uint16_t)(void *)address.word;
						EECR |= (1<<EERE);
						putch(EEDR);

						address.word++;
					}
					else {

						if (!flags.rampz) 
							putch(pgm_read_byte_near(address.word));

						address.word++;
					}
				}

				putch(0x10);
			}
		}

		/* Get device signature bytes  */
		else if(ch=='u') {
			if (getch() == ' ') {
				putch(0x14);
				putch(SIG1);
				putch(SIG2);
				putch(SIG3);
				putch(0x10);
			} else {
				if (++error_count == MAX_ERROR_COUNT)
					app_start();
			}
		}

		/* Read oscillator calibration byte */
		else if(ch=='v') {
			byte_response(0x00);
		}
		else if (++error_count == MAX_ERROR_COUNT) {
			app_start();
		}
	} /* end of forever loop */
}
