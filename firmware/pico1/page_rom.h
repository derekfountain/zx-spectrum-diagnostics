#ifndef __PAGE_ROM_H
#define __PAGE_ROM_H

#include "page.h"

void rom_page_init( void );
void rom_page_entry( void );
void rom_page_gpios( uint32_t gpio, uint32_t events );
void rom_page_run_seq_test( void );
void rom_output(void);
void rom_page_exit( void );

#endif
