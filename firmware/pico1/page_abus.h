#ifndef __PAGE_ABUS_H
#define __PAGE_ABUS_H

#include "page.h"
#include "hardware/pio.h"

void abus_page_init( void );
void abus_page_entry( void );
void abus_page_gpios( uint32_t gpio, uint32_t events );
void abus_page_run_tests( PIO linkin_pio, PIO linkout_pio, int linkin_sm, int linkout_sm );
void abus_output(void);
void abus_page_exit( void );

#endif
