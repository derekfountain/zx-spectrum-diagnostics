/*
 * Z80 control bus tests
 */

#include "oled.h"
#include "page.h"
#include "gpios.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

/* Long enough to let the Spectrum boot and run a decent part of the ROM */
#define TEST_TIME_SECS   5
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))
static uint32_t m1_counter = 0;

alarm_id_t z80_alarm_id = -1;

static bool test_running = false;

static void test_blipper( void )
{
  gpio_put( GPIO_P1_BLIPPER, 1 );
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  gpio_put( GPIO_P1_BLIPPER, 0 );
}

int64_t __time_critical_func(z80_alarm_callback)(alarm_id_t id, void *user_data)
{
  test_running = false;
  return 0;
}

void z80_page_init( void )
{
  /* Set up the Z80 M1 sampling GPIO */
  gpio_init( GPIO_Z80_M1 ); gpio_set_dir( GPIO_Z80_M1, GPIO_IN ); gpio_pull_up( GPIO_Z80_M1 );
}

void z80_page_entry( void )
{
  /* NOP as yet */
}

void z80_page_exit( void )
{
}

void z80_page_gpios( uint32_t gpio, uint32_t events )
{
  if( gpio == GPIO_Z80_M1 )
  {
    if( test_running )
      m1_counter++;
  }
}

void z80_page_run_tests( void )
{
  m1_counter = 0;

  /*
   * Let the Z80 run. The sleep is to let the capacitor C27 in the
   * Spectrum charge up and release the RESET line
   */
  gpio_put( GPIO_Z80_RESET, 0 ); sleep_ms( 650 );

  /* Restart the alarm which defines the duration of the test */
  test_running = true;
  z80_alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, z80_alarm_callback, NULL, false );

  while( test_running );

  /*
   * Alarm is done with, this reset isn't necessary but if I change the
   * timing system something else might need to go here
   */
  cancel_alarm( z80_alarm_id );
  z80_alarm_id = -1;

  /* Stop the Z80 again, we exit these tests with the Z80 held in reset */
  gpio_put( GPIO_Z80_RESET, 1 ); sleep_ms( 650 );

  sleep_ms( 10 );
}


void z80_page_test_m1( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t result_line[32];

  sprintf( result_line, "  M1: %s", m1_counter > 0 ? "     OK" : "Missing" );
  strncpy( result_txt, result_line, result_txt_max_len );
}
