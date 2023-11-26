#ifndef __PAGE_VOLTAGES_H
#define __PAGE_VOLTAGES_H

#include <stdint.h>
#include "page.h"

void voltage_page_entry( void );
void voltage_page_test_5v( uint8_t *result_txt, uint32_t result_txt_max_len );
void voltage_page_test_12v( uint8_t *result_txt, uint32_t result_txt_max_len );
void voltage_page_test_minus5v( uint8_t *result_txt, uint32_t result_txt_max_len );
void voltage_page_exit( void );

#endif
