#ifndef __PAGE_Z80_H
#define __PAGE_Z80_H

#include "page.h"

void z80_page_init( void );
void z80_page_entry( void );
void z80_page_gpios( uint32_t gpio, uint32_t events );
void z80_page_run_tests( void );
void z80_output(void);
void z80_page_exit( void );

#endif
