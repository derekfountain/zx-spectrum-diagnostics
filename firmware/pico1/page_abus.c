/*
 * Address bus tests
 */

#include "oled.h"
#include "page.h"
#include "gpios.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

/* Long enough to let the Spectrum boot and run a decent part of the ROM */
#define TEST_TIME_SECS   2
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))

#define NUM_ABUS_TEST_RESULT_LINES 1
#define WIDTH_OLED_CHARS 32
static uint8_t result_line_txt[NUM_ABUS_TEST_RESULT_LINES][WIDTH_OLED_CHARS+1];

void abus_page_init( void )
{
  /* Set up the GPIO which pokes the other Pico */
  gpio_init( GPIO_P1_SIGNAL ); gpio_set_dir( GPIO_P1_SIGNAL, GPIO_OUT ); gpio_put( GPIO_P1_SIGNAL, 0 );
}

void abus_page_entry( void )
{
gpio_put( GPIO_P1_SIGNAL, 1 );
}

void abus_page_exit( void )
{
gpio_put( GPIO_P1_SIGNAL, 0 );
  snprintf( result_line_txt[0], WIDTH_OLED_CHARS, "abus: Not ready");
}

void abus_page_gpios( uint32_t gpio, uint32_t events )
{
}

void abus_page_run_tests( void )
{
}


void abus_output(void)
{
  uint8_t line=2;
  for( uint32_t test_index=0; test_index<NUM_ABUS_TEST_RESULT_LINES; test_index++ )
  {      
    draw_str(0, line*8, "                         " );
    draw_str(0, line*8, result_line_txt[test_index] );      
	
    line++;
  }  
}
