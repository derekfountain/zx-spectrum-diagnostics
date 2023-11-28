#ifndef __PAGE_ULA_H
#define __PAGE_ULA_H

#include "page.h"

void ula_page_gpios( uint gpio, uint32_t events );

void ula_page_init( void );
void ula_page_entry( void );
void ula_page_run_tests( void );
void ula_page_test_int( uint8_t *result_txt, uint32_t result_txt_max_len );
void ula_page_test_clk( uint8_t *result_txt, uint32_t result_txt_max_len );
void ula_page_test_c_clk( uint8_t *result_txt, uint32_t result_txt_max_len );
void ula_page_exit( void );

#endif
