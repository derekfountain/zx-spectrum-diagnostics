/*
 * Data bus tests
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

#define NUM_DBUS_TEST_RESULT_LINES 3
#define WIDTH_OLED_CHARS 32
static uint8_t result_line_txt[NUM_DBUS_TEST_RESULT_LINES][WIDTH_OLED_CHARS+1];

static SEEN_EDGE d0_flag   = SEEN_NEITHER;
static SEEN_EDGE d1_flag   = SEEN_NEITHER;
static SEEN_EDGE d2_flag   = SEEN_NEITHER;
static SEEN_EDGE d3_flag   = SEEN_NEITHER;
static SEEN_EDGE d4_flag   = SEEN_NEITHER;
static SEEN_EDGE d5_flag   = SEEN_NEITHER;
static SEEN_EDGE d6_flag   = SEEN_NEITHER;
static SEEN_EDGE d7_flag   = SEEN_NEITHER;

static bool dbus_test_running = false;

static int64_t __time_critical_func(dbus_alarm_callback)(alarm_id_t id, void *user_data)
{
  dbus_test_running = false;
  return 0;
}

void dbus_page_init( void )
{
  /* Set up the data bus sampling GPIOs */
  gpio_init( GPIO_DBUS_D0 );   gpio_set_dir( GPIO_DBUS_D0, GPIO_IN ); gpio_pull_up( GPIO_DBUS_D0 );
  gpio_init( GPIO_DBUS_D1 );   gpio_set_dir( GPIO_DBUS_D1, GPIO_IN ); gpio_pull_up( GPIO_DBUS_D1 );
  gpio_init( GPIO_DBUS_D2 );   gpio_set_dir( GPIO_DBUS_D2, GPIO_IN ); gpio_pull_up( GPIO_DBUS_D2 );
  gpio_init( GPIO_DBUS_D3 );   gpio_set_dir( GPIO_DBUS_D3, GPIO_IN ); gpio_pull_up( GPIO_DBUS_D3 );
  gpio_init( GPIO_DBUS_D4 );   gpio_set_dir( GPIO_DBUS_D4, GPIO_IN ); gpio_pull_up( GPIO_DBUS_D4 );
  gpio_init( GPIO_DBUS_D5 );   gpio_set_dir( GPIO_DBUS_D5, GPIO_IN ); gpio_pull_up( GPIO_DBUS_D5 );
  gpio_init( GPIO_DBUS_D6 );   gpio_set_dir( GPIO_DBUS_D6, GPIO_IN ); gpio_pull_up( GPIO_DBUS_D6 );
  gpio_init( GPIO_DBUS_D7 );   gpio_set_dir( GPIO_DBUS_D7, GPIO_IN ); gpio_pull_up( GPIO_DBUS_D7 );
}

void dbus_page_entry( void )
{
  /* Need to hold the Z80 offline while I set these up or they fire too early */
  gpio_put( GPIO_Z80_RESET, 1 ); sleep_ms( 650 );

  d0_flag = SEEN_NEITHER;
  d1_flag = SEEN_NEITHER;
  d2_flag = SEEN_NEITHER;
  d3_flag = SEEN_NEITHER;
  d4_flag = SEEN_NEITHER;
  d5_flag = SEEN_NEITHER;
  d6_flag = SEEN_NEITHER;
  d7_flag = SEEN_NEITHER;

  gpio_set_irq_enabled( GPIO_DBUS_D0, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_DBUS_D1, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_DBUS_D2, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_DBUS_D3, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_DBUS_D4, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_DBUS_D5, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_DBUS_D6, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_DBUS_D7, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
}

void dbus_page_exit( void )
{
  gpio_set_irq_enabled( GPIO_DBUS_D0, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_DBUS_D1, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_DBUS_D2, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_DBUS_D3, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_DBUS_D4, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_DBUS_D5, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_DBUS_D6, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_DBUS_D7, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );

  snprintf( result_line_txt[0], WIDTH_OLED_CHARS, "        76543210" );
  snprintf( result_line_txt[1], WIDTH_OLED_CHARS, "        --------" );
  if( (d0_flag == SEEN_BOTH) &&
      (d1_flag == SEEN_BOTH) &&
      (d2_flag == SEEN_BOTH) &&
      (d3_flag == SEEN_BOTH) &&
      (d4_flag == SEEN_BOTH) &&
      (d5_flag == SEEN_BOTH) &&
      (d6_flag == SEEN_BOTH) &&
      (d7_flag == SEEN_BOTH) )
  {
    /* This would be the norm: transitions low to high and high to low have both been seen */
    snprintf( result_line_txt[2], WIDTH_OLED_CHARS, "Active: YYYYYYYY");
  }
  else
  {
    /*
     * Logic is as follows: if flag is SEEN_NEITHER then there has been no edge is
     * either direction. The line is stuck in whatever state it's currently in, high
     * or low. So just read the GPIO state - that's how it's stuck.
     *
     * If the flag indicates the line has been seen going high, but not seen going
     * low, then it must be stuck in its current state which is high.
     *
     * If the flag indicates the line has been seen going low, but not seen going
     * high, then it must be stuck in its current state which is low.
     *
     * Whichever it is, the current state of the GPIO tells me whether it's stuck
     * high or low.
     */
    snprintf( result_line_txt[2], WIDTH_OLED_CHARS, " Stuck: %c%c%c%c%c%c%c%c",
	      d7_flag == SEEN_BOTH ? '.' : gpio_get( GPIO_DBUS_D7 ) ? 'H' : 'L',
	      d6_flag == SEEN_BOTH ? '.' : gpio_get( GPIO_DBUS_D6 ) ? 'H' : 'L',
	      d5_flag == SEEN_BOTH ? '.' : gpio_get( GPIO_DBUS_D5 ) ? 'H' : 'L',
	      d4_flag == SEEN_BOTH ? '.' : gpio_get( GPIO_DBUS_D4 ) ? 'H' : 'L',
	      d3_flag == SEEN_BOTH ? '.' : gpio_get( GPIO_DBUS_D3 ) ? 'H' : 'L',
	      d2_flag == SEEN_BOTH ? '.' : gpio_get( GPIO_DBUS_D2 ) ? 'H' : 'L',
	      d1_flag == SEEN_BOTH ? '.' : gpio_get( GPIO_DBUS_D1 ) ? 'H' : 'L',
	      d0_flag == SEEN_BOTH ? '.' : gpio_get( GPIO_DBUS_D0 ) ? 'H' : 'L');
  }
}

void dbus_page_gpios( uint32_t gpio, uint32_t events )
{
  if( dbus_test_running )
  {
    if( (gpio >= GPIO_DBUS_D0) && (gpio <= GPIO_DBUS_D7) )
    {
      /*
       * One of the data bus GPIOs has changed state. Note whether it was
       * rising or falling. If it's been seen doing both, it behaviour is
       * confirmed as correct and the GPIO callback is switched off.
       */
      SEEN_EDGE *d_flag_ptr=NULL;

      switch( gpio )
      {
      case GPIO_DBUS_D0:
	d_flag_ptr = &d0_flag;
	break;
      case GPIO_DBUS_D1:
	d_flag_ptr = &d1_flag;
	break;
      case GPIO_DBUS_D2:
	d_flag_ptr = &d2_flag;
	break;
      case GPIO_DBUS_D3:
	d_flag_ptr = &d3_flag;
	break;
      case GPIO_DBUS_D4:
	d_flag_ptr = &d4_flag;
	break;
      case GPIO_DBUS_D5:
	d_flag_ptr = &d5_flag;
	break;
      case GPIO_DBUS_D6:
	d_flag_ptr = &d6_flag;
	break;
      case GPIO_DBUS_D7:
	d_flag_ptr = &d7_flag;
	break;
      }

      if( events | GPIO_IRQ_EDGE_FALL )
      {
	*d_flag_ptr |= SEEN_FALLING;
      }
      if( events | GPIO_IRQ_EDGE_RISE )
      {
	*d_flag_ptr |= SEEN_RISING;
      }

      if( *d_flag_ptr == SEEN_BOTH )
      {
	gpio_set_irq_enabled( gpio, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
      }
    }
    else
    {
      panic("DBUS code received invalid GPIO %d", gpio);
    }
  }
}

void dbus_page_run_tests( void )
{
  dbus_test_running = false;

  /*
   * Reboot the Spectrum.
   */
  gpio_put( GPIO_Z80_RESET, 1 ); sleep_ms( 5 );
  gpio_put( GPIO_Z80_RESET, 0 );

  /*
   * I had a 
   *  sleep_ms( 650 );
   * in here, which isn't really necessary because I want to start monitoring
   * the data bus as soon as the Z80 is ready to run. I don't need to pause.
   * Thing is, that paused effectively stopped the Pico. I think it was
   * because with the 8 data bus GPIOs set up and firing the callback, when
   * the Z80 runs the flood of callbacks overwhelmes the Pico and it stalls
   * before the code below which sets up the alarm. The alarm never happens
   * and the test apparently runs forever.
   * Maybe, not sure, but taking that sleep out fixes it.
   */

  /* Start the alarm which defines the duration of the test */
  dbus_test_running = true;
  alarm_id_t dbus_alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, dbus_alarm_callback, NULL, false );
  if( dbus_alarm_id < 0 )
    panic("No alarms available in DBUS test");

  while( dbus_test_running );

  /*
   * Alarm is done with, this reset isn't necessary but if I change the
   * timing system something else might need to go here
   */
  cancel_alarm( dbus_alarm_id );
}


void dbus_output(void)
{
  uint8_t line=2;
  for( uint32_t test_index=0; test_index<NUM_DBUS_TEST_RESULT_LINES; test_index++ )
  {      
    draw_str(0, line*8, "                         " );
    draw_str(0, line*8, result_line_txt[test_index] );      
	
    line++;
  }  
}
