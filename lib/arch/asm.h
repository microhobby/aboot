
#ifndef __ASM_HEADER__
#define __ASM_HEADER__

#include <avr/pgmspace.h>

void asm_flash_data()
{
    asm volatile(
        "clr	r17             \n\t"	//page_word_count
        "lds	r30,address     \n\t"	//Address of FLASH location (in bytes)
        "lds	r31,address+1	\n\t"
        "ldi	r28,lo8(buff)	\n\t"	//Start of buffer array in RAM
        "ldi	r29,hi8(buff)	\n\t"
        "lds	r24,length      \n\t"	//Length of data to be written (in bytes)
        "lds	r25,length+1	\n\t"
        "length_loop:		    \n\t"	//Main loop, repeat for number of words in block							 							 
        "cpi	r17,0x00	    \n\t"	//If page_word_count=0 then erase page
        "brne	no_page_erase	\n\t"						 
        "wait_spm1:		        \n\t"
        "lds	r16,%0		    \n\t"	//Wait for previous spm to complete
        "andi	r16,1           \n\t"
        "cpi	r16,1           \n\t"
        "breq	wait_spm1       \n\t"
        "ldi	r16,0x03	    \n\t"	//Erase page pointed to by Z
        "sts	%0,r16		    \n\t"
        "spm			        \n\t"							 
        "wait_spm2:		        \n\t"
        "lds	r16,%0		    \n\t"	//Wait for previous spm to complete
        "andi	r16,1           \n\t"
        "cpi	r16,1           \n\t"
        "breq	wait_spm2       \n\t"									 

        "ldi	r16,0x11	    \n\t"	//Re-enable RWW section
        "sts	%0,r16		    \n\t"						 			 
        "spm			        \n\t"
        "no_page_erase:		    \n\t"							 
        "ld	r0,Y+		        \n\t"	//Write 2 bytes into page buffer
        "ld	r1,Y+		        \n\t"							      
        "wait_spm3:		        \n\t"
        "lds	r16,%0		    \n\t"	//Wait for previous spm to complete
        "andi	r16,1           \n\t"
        "cpi	r16,1           \n\t"
        "breq	wait_spm3       \n\t"
        "ldi	r16,0x01	    \n\t"	//Load r0,r1 into FLASH page buffer
        "sts	%0,r16	    	\n\t"
        "spm			        \n\t"
                    
        "inc	r17		        \n\t"	//page_word_count++
        "cpi r17,%1	            \n\t"
        "brlo	same_page	    \n\t"	//Still same page in FLASH
        "write_page:		    \n\t"
        "clr	r17		        \n\t"	//New page, write current one first
        "wait_spm4:		        \n\t"
        "lds	r16,%0		    \n\t"	//Wait for previous spm to complete
        "andi	r16,1           \n\t"
        "cpi	r16,1           \n\t"
        "breq	wait_spm4       \n\t"							 							 
        "ldi	r16,0x05	    \n\t"	//Write page pointed to by Z
        "sts	%0,r16		    \n\t"
        "spm			        \n\t"
        "wait_spm5:		        \n\t"
        "lds	r16,%0		    \n\t"	//Wait for previous spm to complete
        "andi	r16,1           \n\t"
        "cpi	r16,1           \n\t"
        "breq	wait_spm5       \n\t"									 
        "ldi	r16,0x11	    \n\t"	//Re-enable RWW section
        "sts	%0,r16		    \n\t"						 			 
        "spm			        \n\t"					 		 
        "same_page:		        \n\t"							 
        "adiw	r30,2		    \n\t"	//Next word in FLASH
        "sbiw	r24,2		    \n\t"	//length-2
        "breq	final_write	    \n\t"	//Finished
        "rjmp	length_loop	    \n\t"
        "final_write:		    \n\t"
        "cpi	r17,0		    \n\t"
        "breq	block_done	    \n\t"
        "adiw	r24,2		    \n\t"	//length+2, fool above check on length after short page write
        "rjmp	write_page	    \n\t"
        "block_done:		    \n\t"
        "clr	__zero_reg__	\n\t"	//restore zero register

        : "=m" (SPMCSR) : "M" (PAGE_SIZE) : "r0","r16","r17","r24","r25","r28","r29","r30","r31"
    );
}

#endif