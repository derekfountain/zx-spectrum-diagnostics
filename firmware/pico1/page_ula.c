/*
 * INTERRUPT line and ULA->Z80 clock line tests.
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

#define NUM_ULA_TESTS 3
#define WIDTH_OLED_CHARS 32
static uint8_t result_line_txt[NUM_ULA_TESTS][WIDTH_OLED_CHARS+1];

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

static int64_t __time_critical_func(ula_alarm_callback)(alarm_id_t id, void *user_data)
{
  test_running = false;
  return 0;
}

/*
 * Initialise ULA tests. This is called once, when the Pico boots up.
 */
void ula_page_init( void )
{
  /* Set up the Z80 interrupt GPIO */
  gpio_init( GPIO_Z80_INT ); gpio_set_dir( GPIO_Z80_INT, GPIO_IN ); gpio_pull_up( GPIO_Z80_INT );
}

void ula_page_entry( void )
{
  gpio_set_irq_enabled( GPIO_Z80_INT, GPIO_IRQ_EDGE_FALL, true );
}

void ula_page_exit( void )
{
  gpio_set_irq_enabled( GPIO_Z80_INT, GPIO_IRQ_EDGE_FALL, false );

  snprintf( result_line_txt[0], WIDTH_OLED_CHARS, " INT: %0.2fHz", ((float)(interrupt_counter)) / TEST_TIME_SECS_F/2.0 );
  snprintf( result_line_txt[1], WIDTH_OLED_CHARS, " CLK: %0.2fMHz", ((float)(clk_counter)/TEST_TIME_SECS_F) / 1000000.0 );
  snprintf( result_line_txt[2], WIDTH_OLED_CHARS, "cCLK: %0.2fMHz", ((float)(c_clk_counter)/TEST_TIME_SECS_F) / 1000000.0 );
}

void ula_page_gpios( uint32_t gpio, uint32_t events )
{
  /*
   * This is the GPIO callback for this test. It's called when the
   * 50Hz interrupt line fires. Just count them.
   */
  if( gpio == GPIO_Z80_INT )
  {
    if( test_running )
    {
      interrupt_counter++;
    }
  }
}

void ula_page_run_tests( void )
{
  /* Assert and hold Z80 reset for first clock test */
  gpio_put( GPIO_Z80_RESET, 1 );

  /* Hardcoded pio0 for this test, at least for now */
  const PIO pio = pio0;

  /* Load clock counting PIO program into PIO's instruction memory and grab a state machine */
  uint32_t offset      = pio_add_program( pio, &clk_counter_program );
  uint32_t sm_clk      = pio_claim_unused_sm( pio, true );
  pio_sm_config config = clk_counter_program_get_default_config(offset);

  /* Set up state machine to run the clock counter for uncontended (i.e. Z80 held in reset) */
  pio_sm_init( pio, sm_clk, offset, &config );
  clk_counter_program_init( pio, sm_clk, offset, GPIO_Z80_CLK );
  pio_sm_set_enabled( pio, sm_clk, true );

  /*
   * Alarm is used to run the test for a defined period. The callback sets the
   * test running flag to false which breaks the loop.
   *
   * I need to start this on the edge of the INT signal to ensure no rounding errors,
   * so enable the GPIO callback code with a bogus test_running=true, then spin waiting
   * for the counter to get bumped, indicating that the GPIO triggered the callback.
   * At that point I know the INT has just happened, so I reset the counter and start
   * the test. The int_checker is in case the INT signal is faultly, in which case
   * after around 20ms has passed I assume it's not coming and I break out to start the
   * test anyway.
   */
  uint32_t int_checker = 0;
  interrupt_counter = 0;
  test_running = true;
  while( interrupt_counter == 0 && int_checker++ < 25 )
    sleep_ms( 1 );
  test_running = false;

  interrupt_counter = 0;
  test_running = true;
  alarm_id_t ula_alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, ula_alarm_callback, NULL, false );
  if( ula_alarm_id < 0 )
    panic("No alarms available in ULA test");

  /* Bump the PIO to start the program, then wait for the timer alarm */
  pio_sm_put( pio, sm_clk, 1 );
  while( test_running );

  /*
   * The alarm went off and test_running is now false. Interrupts aren't being counted.
   *
   * Bump the PIO again to stop the test and have it deliver the value. The value comes
   * back in 1's complement. A tiny pause is required between poking the statemachine
   * to tell it to prepare the answer, and actually reading the answer. It's only a 
   * couple of PIO instructions, but just occasionally it would get confused if I read
   * the value too quickly.
   */
  pio_sm_put( pio, sm_clk, 0 );
  busy_wait_us_32(100);
  clk_counter = pio_sm_get( pio, sm_clk );
  clk_counter = ~clk_counter;

  /* Done this bit, stop the state machine */
  pio_sm_set_enabled(pio, sm_clk, false);


  /*
   * Now do it again, this time with the Z80 running, which makes the ULA
   * use its contention handling
   */


  /* Reinitialise the state machine for the clock counter for contended (i.e. Z80 free running) */
  pio_sm_init( pio, sm_clk, offset, &config );
  clk_counter_program_init( pio, sm_clk, offset, GPIO_Z80_CLK );
  pio_sm_set_enabled( pio, sm_clk, true );

  /*
   * Let the Z80 run. The sleep is to let the capacitor C27 in the
   * Spectrum charge up and release the RESET line
   */
  gpio_put( GPIO_Z80_RESET, 0 ); sleep_ms( 650 );

  /* Restart the alarm which defines the duration of the test */
  test_running = true;
  ula_alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, ula_alarm_callback, NULL, false );
  if( ula_alarm_id < 0 )
    panic("No alarms available in ULA test");

  /* Kick the state machine to make it start counting from 0 again */
  pio_sm_put( pio, sm_clk, 1 );
  while( test_running );

  /* Fetch contended clock counter */
  pio_sm_put( pio, sm_clk, 0 );
  busy_wait_us_32(100);
  c_clk_counter = pio_sm_get( pio, sm_clk );
  c_clk_counter = ~c_clk_counter;

  /* Stop the state machine and release it */
  pio_sm_set_enabled(pio, sm_clk, false);
  pio_sm_unclaim(pio, sm_clk);

  /* Remove program to let something else use the PIO */
  pio_remove_program(pio, &clk_counter_program, offset );

  /*
   * Alarm is done with, this reset isn't necessary but if I change the
   * timing system something else might need to go here
   */
  cancel_alarm( ula_alarm_id );

  /* We exit these tests with the Z80 running */
}


void ula_output(void)
{
  uint8_t line=2;
  for( uint32_t test_index=0; test_index<NUM_ULA_TESTS; test_index++ )
  {      
    draw_str(0, line*8, "                         " );
    draw_str(0, line*8, result_line_txt[test_index] );      
	
    line++;
  }  
}
