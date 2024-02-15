#ifndef __PAGE_ULA_H
#define __PAGE_ULA_H

#include "page.h"

void ula_page_init( void );
void ula_page_entry( void );
void ula_page_gpios( uint32_t gpio, uint32_t events );
void ula_page_run_tests( void );
void ula_page_test_int( void );
void ula_output(void);
void ula_page_exit( void );

#endif
