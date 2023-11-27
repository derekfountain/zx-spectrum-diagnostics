/*
 * INTERRUPT line and Z80 clock line tests.
 */

#include "oled.h"
#include "page.h"
#include "gpios.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "clk_counter.pio.h"

static PIO pio;
static uint sm_clk;
static uint32_t clk_counter;

#define TEST_TIME_SECS   1
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))
static uint32_t interrupt_counter = 0;
static uint32_t clock_counter = 0;

alarm_id_t alarm_id = -1;

static bool test_running = false;

static void test_blipper( void )
{
  gpio_put( GPIO_BLIPPER, 1 );
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  gpio_put( GPIO_BLIPPER, 0 );
}

int64_t __time_critical_func(alarm_callback)(alarm_id_t id, void *user_data)
{
  test_running = false;
  return 0;
}

/*
 * Initialise ULA tests. This is called once, when the Pico boots up.
 * There's a PIO program for the clock test which is put in place here.
 */
void ula_page_init( void )
{
  pio              = pio0;
  sm_clk           = pio_claim_unused_sm( pio, true );
  uint offset_clk  = pio_add_program( pio, &clk_counter_program );
  clk_counter_program_init( pio, sm_clk, offset_clk, GPIO_Z80_CLK );
  pio_sm_set_enabled( pio, sm_clk, false );
}

void ula_page_entry( void )
{
  /* Assert and hold Z80 reset for this test page */
  gpio_put( GPIO_Z80_RESET, 1 );

  /* Set up the Z80 interrupt GPIO */
  gpio_init( GPIO_Z80_INT ); gpio_set_dir( GPIO_Z80_INT, GPIO_IN ); gpio_pull_up( GPIO_Z80_INT );
}

void ula_page_exit( void )
{
  /* Turn off the PIO clock counter */
  pio_sm_set_enabled( pio, sm_clk, false );

  /* This will already be off, no harm doing it again */
  if( alarm_id != -1 )
  {
    cancel_alarm( alarm_id );
    alarm_id = -1;
    test_running = false;
  }

  /* Let the Z80 run again */
  gpio_put( GPIO_Z80_RESET, 0 );  
}

void ula_page_gpios( uint gpio, uint32_t events )
{
  /*
   * This is the GPIO callback for this test. It's called when the
   * 50Hz interrupt line fires. Just count them.
   */
  if( gpio == GPIO_Z80_INT )
  {
    if( test_running )
      interrupt_counter++;
  }
}

void ula_page_run_tests( void )
{
  interrupt_counter = 0;
  clk_counter = 0;

  test_running = true;
  pio_sm_set_enabled(pio, sm_clk, true);
  alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, alarm_callback, NULL, false );

  test_blipper();
  while( test_running );
  test_blipper();

  pio_sm_put( pio, sm_clk, 0 );
  clk_counter = pio_sm_get( pio, sm_clk );
  clk_counter = ~clk_counter;

  pio_sm_set_enabled( pio, sm_clk, false );
  cancel_alarm( alarm_id );
  alarm_id = -1;
}


void ula_page_test_int( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t interrupt_line[32];

  sprintf( interrupt_line, " INT: %0.2fHz", ((float)(interrupt_counter)) / TEST_TIME_SECS );
  strncpy( result_txt, interrupt_line, result_txt_max_len );
}

void ula_page_test_clk( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t clock_line[32];

//  sprintf( clock_line, " CLK: %d", clk_counter/TEST_TIME_SECS );
  sprintf( clock_line, " CLK: %0.2fMHz", ((float)(clk_counter)/TEST_TIME_SECS_F) / 1000000.0 );
//  sprintf( clock_line, " CLK: %d", clk_counter/TEST_TIME_SECS );
  strncpy( result_txt, clock_line, result_txt_max_len );
}

