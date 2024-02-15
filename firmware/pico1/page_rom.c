/*
 * ROM tests
 */

#include "oled.h"
#include "page.h"
#include "gpios.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#define NUM_ROM_TESTS 1
#define WIDTH_OLED_CHARS 32
static uint8_t result_line_txt[NUM_ROM_TESTS][WIDTH_OLED_CHARS+1];

/* Long enough to let the Spectrum boot and run a decent part of the ROM */
#define TEST_TIME_SECS   2
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))

static bool rom_test_running = false;

static int64_t __time_critical_func(dbus_alarm_callback)(alarm_id_t id, void *user_data)
{
  rom_test_running = false;
  return 0;
}

void rom_page_init( void )
{
}

void rom_page_entry( void )
{
  /* Need to hold the Z80 offline while I set these up or they fire too early */
  gpio_put( GPIO_Z80_RESET, 1 ); sleep_ms( 650 );

}

void rom_page_exit( void )
{
}

void rom_page_gpios( uint32_t gpio, uint32_t events )
{
  if( rom_test_running )
  {
  }
}

void rom_page_run_seq_test( void )
{
  snprintf( result_line_txt[0], WIDTH_OLED_CHARS, " Seq: Not ready");
}



void rom_output(void)
{
  uint8_t line=2;
  for( uint32_t test_index=0; test_index<NUM_ROM_TESTS; test_index++ )
  {      
    draw_str(0, line*8, "                         " );
    draw_str(0, line*8, result_line_txt[test_index] );      
	
    line++;
  }  
}
