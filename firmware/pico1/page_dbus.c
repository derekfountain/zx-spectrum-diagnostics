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
#define TEST_TIME_SECS   3
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))

static SEEN_EDGE d0_flag   = SEEN_NEITHER;
static SEEN_EDGE d1_flag   = SEEN_NEITHER;
static SEEN_EDGE d2_flag   = SEEN_NEITHER;
static SEEN_EDGE d3_flag   = SEEN_NEITHER;
static SEEN_EDGE d4_flag   = SEEN_NEITHER;
static SEEN_EDGE d5_flag   = SEEN_NEITHER;
static SEEN_EDGE d6_flag   = SEEN_NEITHER;
static SEEN_EDGE d7_flag   = SEEN_NEITHER;

static bool dbus_test_running = false;

int64_t __time_critical_func(dbus_alarm_callback)(alarm_id_t id, void *user_data)
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

  d0_flag   = SEEN_NEITHER;
  d1_flag   = SEEN_NEITHER;
  d2_flag   = SEEN_NEITHER;
  d3_flag   = SEEN_NEITHER;
  d4_flag   = SEEN_NEITHER;
  d5_flag   = SEEN_NEITHER;
  d6_flag   = SEEN_NEITHER;
  d7_flag   = SEEN_NEITHER;

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
}

void dbus_page_gpios( uint32_t gpio, uint32_t events )
{
  if( dbus_test_running )
  {
    if( (gpio >= GPIO_DBUS_D0) && (gpio <= GPIO_DBUS_D7) )
    {
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
  }
}

void dbus_page_run_tests( void )
{
  /*
   * Allow the Z80 to run. Test starts immediately.
   */
  gpio_put( GPIO_Z80_RESET, 0 );

  /* Restart the alarm which defines the duration of the test */
  dbus_test_running = true;
  alarm_id_t dbus_alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, dbus_alarm_callback, NULL, false );

  while( dbus_test_running );

  /*
   * Alarm is done with, this reset isn't necessary but if I change the
   * timing system something else might need to go here
   */
  cancel_alarm( dbus_alarm_id );
  dbus_alarm_id = -1;
}


void dbus_page_test_result( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t result_line[32];

  d3_flag = SEEN_NEITHER;
  sprintf( result_line, "Bus: %c %c %c %c %c %c %c %c",
                        d0_flag == SEEN_BOTH ? 'Y' : ' ',
                        d1_flag == SEEN_BOTH ? 'Y' : ' ',
                        d2_flag == SEEN_BOTH ? 'Y' : ' ',
                        d3_flag == SEEN_BOTH ? 'Y' : ' ',
                        d4_flag == SEEN_BOTH ? 'Y' : ' ',
                        d5_flag == SEEN_BOTH ? 'Y' : ' ',
                        d6_flag == SEEN_BOTH ? 'Y' : ' ',
                        d7_flag == SEEN_BOTH ? 'Y' : ' ' );
  strncpy( result_txt, result_line, result_txt_max_len );
}
