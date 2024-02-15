#ifndef __PAGE_VOLTAGES_H
#define __PAGE_VOLTAGES_H

#include <stdint.h>
#include "page.h"

void voltage_page_init( void );
void voltage_page_entry( void );
void voltage_page_test_5v( void );
void voltage_page_test_12v( void );
void voltage_page_test_minus5v( void );
void voltage_output(void);
void voltage_page_exit( void );

#endif
