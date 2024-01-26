#ifndef __PAGE_Z80_H
#define __PAGE_Z80_H

#include "page.h"

void z80_page_init( void );
void z80_page_entry( void );
void z80_page_gpios( uint32_t gpio, uint32_t events );
void z80_page_run_tests( void );
void z80_page_test_m1( uint8_t *result_txt, uint32_t result_txt_max_len );
void z80_page_test_rd( uint8_t *result_txt, uint32_t result_txt_max_len );
void z80_page_test_wr( uint8_t *result_txt, uint32_t result_txt_max_len );
void z80_page_exit( void );

#endif
