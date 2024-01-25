#include <string.h>

#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#include "page.h"
#include "oled.h"
#include "gpios.h"

#include "page_voltages.h"
#include "page_ula.h"

static uint8_t input1_pressed = 0;
static uint8_t input2_pressed = 0;

auto_init_mutex( oled_mutex );

typedef enum
{
  VOLTAGE_PAGE = 0,
  ULA_PAGE,
  Z80_PAGE,

  LAST_PAGE = ULA_PAGE
}
PAGE;

PAGE current_page;

typedef struct
{
  PAGE page;
  uint32_t first_displayed_test;
  uint32_t last_displayed_test;
}
DISPLAY_PAGE;

DISPLAY_PAGE page[] =
{
  { VOLTAGE_PAGE, 0, 2 },
  { ULA_PAGE,     3, 5 },
};


#define NUM_TESTS        6
#define WIDTH_OLED_CHARS 32
static uint8_t result_line_txt[NUM_TESTS][WIDTH_OLED_CHARS];

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
      {
	input1_pressed = 1;
      }
      else if( gpio == GPIO_INPUT2 )
      {
	input2_pressed = 1;
      }

      /* Note this point as when we last actioned a switch */
      debounce_timestamp_us = get_time_us();
    }
  }
  else
  {
    ula_page_gpios( gpio, events );
  }
}


static void core1_main( void )
{
  uint8_t result_line[WIDTH_OLED_CHARS];

  ula_page_init();

  while( 1 )
  {
    uint8_t test_num = 0;

    /***
     *     __   __     _  _                        
     *     \ \ / /___ | || |_  __ _  __ _  ___  ___
     *      \ V // _ \| ||  _|/ _` |/ _` |/ -_)(_-<
     *       \_/ \___/|_| \__|\__,_|\__, |\___|/__/
     *                              |___/          
     */

    /* Initialise the voltage tests */
    voltage_page_entry();

    /* Run the 5V rail test and populate the result line for the display */
    voltage_page_test_5v( result_line, WIDTH_OLED_CHARS );
    mutex_enter_blocking( &oled_mutex );      
    strncpy( result_line_txt[test_num++], result_line, WIDTH_OLED_CHARS );
    mutex_exit( &oled_mutex );      

    /* Run the 12V rail test and populate the result line for the display */
    voltage_page_test_12v( result_line, WIDTH_OLED_CHARS );
    mutex_enter_blocking( &oled_mutex );      
    strncpy( result_line_txt[test_num++], result_line, WIDTH_OLED_CHARS );
    mutex_exit( &oled_mutex );      
      
    /* Run the -5V rail test and populate the result line for the display */
    voltage_page_test_minus5v( result_line, WIDTH_OLED_CHARS );
    mutex_enter_blocking( &oled_mutex );      
    strncpy( result_line_txt[test_num++], result_line, WIDTH_OLED_CHARS );
    mutex_exit( &oled_mutex );      

    /* Tear down voltage tests */
    voltage_page_exit();

    /***
     *      _   _  _       _   
     *     | | | || |     /_\  
     *     | |_| || |__  / _ \ 
     *      \___/ |____|/_/ \_\
     *                         
     */

    /* Initialise the ULA tests */
    ula_page_entry();

    /* The ULA tests are all run in one function */
    ula_page_run_tests();
    
    /* Pick up the interrupt test result and populate the result line for the display */
    ula_page_test_int( result_line, WIDTH_OLED_CHARS );
    mutex_enter_blocking( &oled_mutex );      
    strncpy( result_line_txt[test_num++], result_line, WIDTH_OLED_CHARS );
    mutex_exit( &oled_mutex );      

    /* Pick up the clock test result and populate the result line for the display */
    ula_page_test_clk( result_line, WIDTH_OLED_CHARS );
    mutex_enter_blocking( &oled_mutex );      
    strncpy( result_line_txt[test_num++], result_line, WIDTH_OLED_CHARS );
    mutex_exit( &oled_mutex );      

    /* Pick up the contended clock test result and populate the result line for the display */
    ula_page_test_c_clk( result_line, WIDTH_OLED_CHARS );
    mutex_enter_blocking( &oled_mutex );      
    strncpy( result_line_txt[test_num++], result_line, WIDTH_OLED_CHARS );
    mutex_exit( &oled_mutex );      

    /* Tear down ULA tests */
    ula_page_exit();


    /* End of tests, pause a while then do them again */
    sleep_ms( 100 );
  }
}

void main( void )
{
  bi_decl(bi_program_description("ZX Spectrum Diagnostics Pico1 Board Binary."));

  sleep_ms( 1000 );

  uint8_t line;
  for( line=0; line<NUM_TESTS; line++ )
    result_line_txt[line][0] = '\0';

  /* Initialise OLED screen with default font */
  init_oled( NULL );

  /* Blipper, for the scope */
  gpio_init( GPIO_P1_BLIPPER ); gpio_set_dir( GPIO_P1_BLIPPER, GPIO_OUT ); gpio_put( GPIO_P1_BLIPPER, 0 );

  /* Start with the Z80 not running */
  gpio_init( GPIO_Z80_RESET ); gpio_set_dir( GPIO_Z80_RESET, GPIO_OUT ); gpio_put( GPIO_Z80_RESET, 1 );

  /* Switch button GPIOs */
  gpio_init( GPIO_INPUT1 ); gpio_set_dir( GPIO_INPUT1, GPIO_IN ); gpio_pull_up( GPIO_INPUT1 );
  gpio_init( GPIO_INPUT2 ); gpio_set_dir( GPIO_INPUT2, GPIO_IN ); gpio_pull_up( GPIO_INPUT2 );

  gpio_set_irq_enabled_with_callback( GPIO_INPUT1, GPIO_IRQ_EDGE_FALL, true, &gpios_callback );
  gpio_set_irq_enabled( GPIO_INPUT2, GPIO_IRQ_EDGE_FALL, true );

  gpio_set_irq_enabled( GPIO_Z80_INT, GPIO_IRQ_EDGE_FALL, true );

  /* Init complete, run 2nd core code */
  multicore_launch_core1( core1_main ); 

  current_page = VOLTAGE_PAGE;

  /* Main loop just loops over the result text lines displaying them */
  while( 1 )
  {
    if( input1_pressed )
    {
      if( current_page++ == LAST_PAGE )
	current_page = VOLTAGE_PAGE;

      input1_pressed = 0;
    }

    uint8_t line=0;
    for( uint32_t test_index=page[current_page].first_displayed_test; test_index<=page[current_page].last_displayed_test; test_index++ )
    {      
      draw_str(0, line*8, "                         " );
      mutex_enter_blocking( &oled_mutex );      
      draw_str(0, line*8, result_line_txt[test_index] );      
      mutex_exit( &oled_mutex );      

      line++;
    }
    update_screen();    

    sleep_ms( 100 );
  }

}
