/*
 * ROM tests
 */

#include "oled.h"
#include "page.h"
#include "gpios.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#include "link_common.h"

#define NUM_ROM_TESTS 1
#define WIDTH_OLED_CHARS 32
static uint8_t result_line_txt[NUM_ROM_TESTS][WIDTH_OLED_CHARS+1];

/* Long enough to let the Spectrum boot and run a decent part of the ROM */
#define TEST_TIME_SECS   2
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))

#define PICO_COMM_TEST_ROM  0x04030201

static bool rom_test_running = false;

static int64_t __time_critical_func(rom_alarm_callback)(alarm_id_t id, void *user_data)
{
  rom_test_running = false;
  return 0;
}

void rom_page_init( void )
{
}

void rom_page_entry( void )
{
}

void rom_page_exit( void )
{
}

void rom_page_gpios( uint32_t gpio, uint32_t events )
{
}

void rom_page_run_seq_test( PIO linkin_pio, PIO linkout_pio, int linkin_sm, int linkout_sm )
{
#if 0
  /* LED can be useful for this one */
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
#endif

  rom_test_running = false;

  /*
   * Reboot the Spectrum.
   */
  gpio_put( GPIO_Z80_RESET, 1 ); sleep_ms( 5 );
  gpio_put( GPIO_Z80_RESET, 0 );

  /* Tell the other Pico which test to run */
  uint32_t test_type = PICO_COMM_TEST_ROM;
  ui_link_send_buffer( linkout_pio, linkout_sm, linkin_sm, (uint8_t*)&test_type, sizeof(test_type) );

  /* Flag the other Pico, which monitors the address bus */
  gpio_put( GPIO_P1_SIGNAL, 1 );
  // gpio_put(LED_PIN, 1);

  /* Start the alarm which defines the duration of the test */
  rom_test_running = true;
  alarm_id_t rom_alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, rom_alarm_callback, NULL, false );
  if( rom_alarm_id < 0 )
    panic("No alarms available in ROM test");

  while( rom_test_running );

  /* Remove flag to stop the other Pico collecting address data */
  gpio_put( GPIO_P1_SIGNAL, 0 );
  //gpio_put(LED_PIN, 0);

  /*
   * Alarm is done with, this reset isn't necessary but if I change the
   * timing system something else might need to go here
   */
  cancel_alarm( rom_alarm_id );

  uint32_t dummy_result;
  ui_link_receive_buffer( linkin_pio, linkin_sm, linkout_sm, (uint8_t*)&dummy_result, sizeof(dummy_result) );

  /* Show the result line */
  snprintf( result_line_txt[0], WIDTH_OLED_CHARS, " ROM: Not ready %u", dummy_result );

  /*
   * Repeat test a regular intervals, but don't do it too fast in case the comms
   * goes a bit funny. No reason it should, but let's not tempt it
   */
  sleep_ms(1000);
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
