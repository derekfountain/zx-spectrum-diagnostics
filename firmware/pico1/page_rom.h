#ifndef __PAGE_ROM_H
#define __PAGE_ROM_H

#include "page.h"
#include "hardware/pio.h"

void rom_page_init( void );
void rom_page_entry( void );
void rom_page_gpios( uint32_t gpio, uint32_t events );
void rom_page_run_seq_test( PIO linkin_pio, PIO linkout_pio, int linkin_sm, int linkout_sm );
void rom_output(void);
void rom_page_exit( void );

#endif
