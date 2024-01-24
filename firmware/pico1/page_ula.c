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

static uint32_t clk_counter;
static uint32_t c_clk_counter;

#define TEST_TIME_SECS   2
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))
static uint32_t interrupt_counter = 0;
static uint32_t clock_counter     = 0;

alarm_id_t alarm_id = -1;

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

int64_t __time_critical_func(alarm_callback)(alarm_id_t id, void *user_data)
{
  test_running = false;
  return 0;
}

/*
 * Initialise ULA tests. This is called once, when the Pico boots up.
 */
void ula_page_init( void )
{
  /* NOP as yet */
}

void ula_page_entry( void )
{
  /* Set up the Z80 interrupt GPIO */
  gpio_init( GPIO_Z80_INT ); gpio_set_dir( GPIO_Z80_INT, GPIO_IN ); gpio_pull_up( GPIO_Z80_INT );
}

void ula_page_exit( void )
{
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

  /* Assert and hold Z80 reset for first clock test */
  gpio_put( GPIO_Z80_RESET, 1 );

  const PIO pio = pio0;

  /* Load clock counting PIO program into PIO's instruction memory */
  uint32_t offset  = pio_add_program( pio, &clk_counter_program );

  /* Set up state machine to run the clock counter for uncontended (i.e. Z80 held in reset) */
  uint32_t sm_clk  = pio_claim_unused_sm( pio, true );
  clk_counter_program_init( pio, sm_clk, offset, GPIO_Z80_CLK );
  pio_sm_exec( pio, sm_clk, pio_encode_jmp(offset) );
  pio_sm_set_enabled( pio, sm_clk, true );

  /*
   * Alarm is used to run the test for a defined period. The callback sets the
   * test running flag to false which breaks the loop
   */
// I need to start this on the edge of the INT signal to ensure no rounding errors
  test_running = true;
  alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, alarm_callback, NULL, false );

  /* Bump the PIO to start the program, then wait for the timer alarm */
  pio_sm_put( pio, sm_clk, 1 );
  while( test_running );

  /*
   * The alarm went off and test_running is now false. Interrupts aren't beiogn counted.
   *
   * Bump the PIO again to stop the test and have it deliver the value. The value comes
   * back in 1's complement
   */
  pio_sm_put( pio, sm_clk, 0 );
  clk_counter = pio_sm_get( pio, sm_clk );
  clk_counter = ~clk_counter;

  pio_sm_set_enabled(pio, sm_clk, false);
  pio_sm_clear_fifos(pio, sm_clk);
  pio_sm_unclaim(pio, sm_clk);


  /*
   * Now do it again, this time with the Z80 running, which makes the ULA
   * use its contention handling
   */

  /*
   * Let the Z80 run. The sleep is to let the capacitor C27 in the
   * Spectrum charge up and release the RESET line
   */
  gpio_put( GPIO_Z80_RESET, 0 ); sleep_ms( 650 );

  /* State machine runs the clock counter for contended (i.e. Z80 free running) */
  sm_clk  = pio_claim_unused_sm( pio, true );
  clk_counter_program_init( pio, sm_clk, offset, GPIO_Z80_CLK );
  pio_sm_set_enabled( pio, sm_clk, true );

  test_running = true;
  alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, alarm_callback, NULL, false );

  pio_sm_put( pio, sm_clk, 1 );
  while( test_running );

  pio_sm_put( pio, sm_clk, 0 );
  c_clk_counter = pio_sm_get( pio, sm_clk );
  c_clk_counter = ~c_clk_counter;

  pio_sm_set_enabled(pio, sm_clk, false);
  pio_sm_clear_fifos(pio, sm_clk);
  pio_sm_unclaim(pio, sm_clk);

  /* Remove program to let something else use the PIO */
  pio_remove_program(pio, &clk_counter_program, offset );

  cancel_alarm( alarm_id );
  alarm_id = -1;

  /* Stop the Z80 again */
  gpio_put( GPIO_Z80_RESET, 1 );
  sleep_ms( 100 );
}


void ula_page_test_int( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t interrupt_line[32];

  /*
   * The interrupt counter test runs while the non-contended and contended tests
   * run. i.e. twice. I could make it run just the once, but it's easier to 
   * divide the result by 2 here. :)
   */
  sprintf( interrupt_line, " INT: %0.2fHz", ((float)(interrupt_counter)) / TEST_TIME_SECS_F/2.0 );
  strncpy( result_txt, interrupt_line, result_txt_max_len );
}

void ula_page_test_clk( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t clock_line[32];

  sprintf( clock_line, " CLK: %0.2fMHz", ((float)(clk_counter)/TEST_TIME_SECS_F) / 1000000.0 );
  strncpy( result_txt, clock_line, result_txt_max_len );
}

void ula_page_test_c_clk( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t clock_line[32];

  sprintf( clock_line, "cCLK: %0.2fMHz", ((float)(c_clk_counter)/TEST_TIME_SECS_F) / 1000000.0 );
  strncpy( result_txt, clock_line, result_txt_max_len );
}

