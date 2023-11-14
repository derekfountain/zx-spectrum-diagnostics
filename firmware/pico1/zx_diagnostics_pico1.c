#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "page.h"
#include "oled.h"
#include "gpios.h"

#include "page_voltages.h"

static page_t *current_page;

static uint8_t input1_pressed;
static uint8_t input2_pressed;

/* From the timer_lowlevel.c example */
static uint64_t get_time_us( void )
{
  uint32_t lo = timer_hw->timelr;
  uint32_t hi = timer_hw->timehr;
  return ((uint64_t)hi << 32u) | lo;
}
static uint64_t debounce_timestamp_us = 0;
void gpios_callback( uint gpio, uint32_t events ) 
{
  /*
   * Switch event, set to interrupt on falling edge, so this is a click down.
   */
  if( (gpio == GPIO_INPUT1) || (gpio == GPIO_INPUT2) )
  {
#define DEBOUNCE_USECS 100000
    /* Debounce pause, the switch is a bit noisy */
    if( (get_time_us() - debounce_timestamp_us) < DEBOUNCE_USECS )
    {
      /* If last switch action was very recently, assume it's a bounce and ignore it */
      debounce_timestamp_us = get_time_us();
    }
    else
    {
      /* Wait for the switch to come back up, then let it settle */
      while( gpio_get( gpio ) );
      busy_wait_us_32( DEBOUNCE_USECS );

      /* Debounced, take action - just set a flag */
      if( gpio == GPIO_INPUT1 )
	input1_pressed;
      else if( gpio == GPIO_INPUT2 )
	input2_pressed;

      /* Note this point as when we last actioned a switch */
      debounce_timestamp_us = get_time_us();
    }
  }
}


void main( void )
{
  bi_decl(bi_program_description("ZX Spectrum Diagnostics Pico1 Board Binary."));

  /* Initialise OLED screen with default font */
  init_oled( NULL );
  
  gpio_init( GPIO_INPUT1 ); gpio_set_dir( GPIO_INPUT1, GPIO_IN ); gpio_pull_up( GPIO_INPUT1 );
  gpio_init( GPIO_INPUT2 ); gpio_set_dir( GPIO_INPUT2, GPIO_IN ); gpio_pull_up( GPIO_INPUT2 );

  gpio_set_irq_enabled_with_callback( GPIO_INPUT1, GPIO_IRQ_EDGE_FALL, true, &gpios_callback );
  gpio_set_irq_enabled( GPIO_INPUT2, GPIO_IRQ_EDGE_FALL, true );

  current_page = get_voltage_page();
  (current_page->entry_fn)();

  while( 1 )
  {
    // This can't take too long. If it turns out that tests need to
    // run a long time, this will have to be done on the second core
    //
    (current_page->tests_fn)();

    for( uint32_t i=0; i < current_page->repeat_pause_ms; i++ )
    {
      if( input1_pressed )
      {
	input1_pressed = 0;
	(current_page->exit_fn)();
	// current_page = need to call function which returns the next page
	(current_page->entry_fn)();
	break;
      }
      else
      {
	sleep_ms( 1 );
      }
    }

  }

}
