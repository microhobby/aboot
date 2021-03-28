/**
 * DEFAULT DEFINES FOR ABOOT
 **/

/* ABOOT */

#define F_CPU               16000000L
#define MAX_ERROR_COUNT     5
#define MAX_TIME_COUNT      F_CPU>>4
#define NUM_LED_FLASHES     3
#define BAUD_RATE           57600

/* SW_MAJOR and MINOR needs to be updated */
/* never allow AVR Studio to do an update !!!! */
#define HW_VER	            0x02
#define SW_MAJOR            0x04
#define SW_MINOR            0x04

/* for ATMEGA328P */
#define LED_DDR             DDRB
#define LED_PORT            PORTB
#define LED_PIN             PINB
#define LED                 PINB5

/* UART */
#define BL_DDR              DDRD
#define BL_PORT             PORTD
#define BL_PIN              PIND
#define BL                  PIND6

/* ATMEL */
#define SIG1	            0x1E
#define SIG2	            0x95
#define SIG3	            0x0F
#define PAGE_SIZE	0x40U	//64 words
