/*
 * INT line.
 * This runs once as the test page is entered. Need to do better.
 */

#include "oled.h"
#include "page.h"
#include "gpios.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#define INT_TEST_TIME_SECS   2
#define INT_TEST_TIME_SECS_F ((float)(INT_TEST_TIME_SECS_F))
static uint32_t int_counter = 0;

/* Tidy this up a bit, then add a check for the Z80's CLK signal.
   Hold the Z80 offline, the clock should be 3.5MHz
   With the Z80 running the clock might be less than 3.5MHz
   because the ULA stops it when it's accessing system variables
   and stuff. Not sure. Might be worth checking what the average is*/

alarm_id_t int_alarm_id = -1;

static bool int_test_running = false;

void ula_page_entry( void )
{
  /* Assert and hold Z80 reset for this test page */
  gpio_put( GPIO_Z80_RESET, 1 );

  gpio_init( GPIO_Z80_INT ); gpio_set_dir( GPIO_Z80_INT, GPIO_IN ); gpio_pull_up( GPIO_Z80_INT );
}

void ula_page_exit( void )
{
  if( int_alarm_id != -1 )
    cancel_alarm( int_alarm_id );

  /* Let the Z80 run again */
  gpio_put( GPIO_Z80_RESET, 0 );  
}

void ula_page_gpios( uint gpio, uint32_t events )
{
  if( gpio == GPIO_Z80_INT )
  {
    if( int_test_running )
      int_counter++;
  }
}

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
  int_test_running = false;
  return 0;
}

void ula_page_test_int( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t int_line[32];

  int_counter = 0;

  int_test_running = true;
  int_alarm_id = add_alarm_in_ms( INT_TEST_TIME_SECS*1000, alarm_callback, NULL, false );  

  while( int_test_running )
    sleep_ms(1);

  cancel_alarm( int_alarm_id );
  int_alarm_id = -1;

  sprintf( int_line, " INT: %0.2fHz", ((float)(int_counter)) / INT_TEST_TIME_SECS );
  strncpy( result_txt, int_line, result_txt_max_len );
}

