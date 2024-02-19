#ifndef __PAGE_ABUS_H
#define __PAGE_ABUS_H

#include "page.h"

void abus_page_init( void );
void abus_page_entry( void );
void abus_page_gpios( uint32_t gpio, uint32_t events );
void abus_page_run_tests( void );
void abus_output(void);
void abus_page_exit( void );

#endif
