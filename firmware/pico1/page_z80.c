/*
 * Z80 control bus tests
 */

#include "oled.h"
#include "page.h"
#include "gpios.h"
#include "test_data.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

/*
 * Long enough to let the Spectrum boot and run a decent part of the ROM.
 * IORQ only changes when the Spectrum gets as far as changing the
 * border, which takes a couple of seconds
 */
#define TEST_TIME_SECS   2
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))

static SEEN_EDGE m1_flag   = SEEN_NEITHER;
static SEEN_EDGE rd_flag   = SEEN_NEITHER;
static SEEN_EDGE wr_flag   = SEEN_NEITHER;
static SEEN_EDGE mreq_flag = SEEN_NEITHER;
static SEEN_EDGE iorq_flag = SEEN_NEITHER;

#define NUM_Z80_TESTS 5
#define WIDTH_OLED_CHARS 32
static uint8_t result_line_txt[NUM_Z80_TESTS][WIDTH_OLED_CHARS+1];

static bool z80_test_running = false;

static int64_t __time_critical_func(z80_alarm_callback)(alarm_id_t id, void *user_data)
{
  z80_test_running = false;
  return 0;
}

void z80_page_init( void )
{
  /* Set up the Z80 sampling GPIOs */
  gpio_init( GPIO_Z80_M1 );   gpio_set_dir( GPIO_Z80_M1,   GPIO_IN ); gpio_pull_up( GPIO_Z80_M1 );
  gpio_init( GPIO_Z80_RD );   gpio_set_dir( GPIO_Z80_RD,   GPIO_IN ); gpio_pull_up( GPIO_Z80_RD );
  gpio_init( GPIO_Z80_WR );   gpio_set_dir( GPIO_Z80_WR,   GPIO_IN ); gpio_pull_up( GPIO_Z80_WR );
  gpio_init( GPIO_Z80_MREQ ); gpio_set_dir( GPIO_Z80_MREQ, GPIO_IN ); gpio_pull_up( GPIO_Z80_MREQ );
  gpio_init( GPIO_Z80_IORQ ); gpio_set_dir( GPIO_Z80_IORQ, GPIO_IN ); gpio_pull_up( GPIO_Z80_IORQ );
}

void z80_page_entry( void )
{
  m1_flag   = SEEN_NEITHER;
  rd_flag   = SEEN_NEITHER;
  wr_flag   = SEEN_NEITHER;
  mreq_flag = SEEN_NEITHER;
  iorq_flag = SEEN_NEITHER;

  gpio_set_irq_enabled( GPIO_Z80_M1,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_Z80_RD,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_Z80_WR,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_Z80_MREQ, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_Z80_IORQ, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
}

void z80_page_exit( void )
{
  gpio_set_irq_enabled( GPIO_Z80_M1,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_Z80_RD,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_Z80_WR,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_Z80_MREQ, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_Z80_IORQ, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );

  snprintf( result_line_txt[0], WIDTH_OLED_CHARS, "  M1: %s", m1_flag   == SEEN_BOTH ? "OK" : "Inactive" );
  snprintf( result_line_txt[1], WIDTH_OLED_CHARS, "  RD: %s", rd_flag   == SEEN_BOTH ? "OK" : "Inactive" );
  snprintf( result_line_txt[2], WIDTH_OLED_CHARS, "  WR: %s", wr_flag   == SEEN_BOTH ? "OK" : "Inactive" );
  snprintf( result_line_txt[3], WIDTH_OLED_CHARS, "MREQ: %s", mreq_flag == SEEN_BOTH ? "OK" : "Inactive" );
  snprintf( result_line_txt[4], WIDTH_OLED_CHARS, "IORQ: %s", iorq_flag == SEEN_BOTH ? "OK" : "Inactive" );
}

void z80_page_gpios( uint32_t gpio, uint32_t events )
{
  if( z80_test_running )
  {
    switch( gpio )
    {
    case GPIO_Z80_M1:
    {
      if( events | GPIO_IRQ_EDGE_FALL )
      {
	m1_flag |= SEEN_FALLING;
      }
      if( events | GPIO_IRQ_EDGE_RISE )
      {
	m1_flag |= SEEN_RISING;
      }

      if( m1_flag == SEEN_BOTH )
      {
	gpio_set_irq_enabled( GPIO_Z80_M1,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
      }
    }
    break;

    case GPIO_Z80_RD:
    {
      if( events | GPIO_IRQ_EDGE_FALL )
      {
	rd_flag |= SEEN_FALLING;
      }
      if( events | GPIO_IRQ_EDGE_RISE )
      {
	rd_flag |= SEEN_RISING;
      }

      if( rd_flag == SEEN_BOTH )
      {
	gpio_set_irq_enabled( GPIO_Z80_RD,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
      }
    }
    break;

    case GPIO_Z80_WR:
    {
      if( events | GPIO_IRQ_EDGE_FALL )
      {
	wr_flag |= SEEN_FALLING;
      }
      if( events | GPIO_IRQ_EDGE_RISE )
      {
	wr_flag |= SEEN_RISING;
      }

      if( wr_flag == SEEN_BOTH )
      {
	gpio_set_irq_enabled( GPIO_Z80_WR,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
      }
    }
    break;

    case GPIO_Z80_MREQ:
    {
      if( events | GPIO_IRQ_EDGE_FALL )
      {
	mreq_flag |= SEEN_FALLING;
      }
      if( events | GPIO_IRQ_EDGE_RISE )
      {
	mreq_flag |= SEEN_RISING;
      }

      if( mreq_flag == SEEN_BOTH )
      {
	gpio_set_irq_enabled( GPIO_Z80_MREQ,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
      }
    }
    break;

    case GPIO_Z80_IORQ:
    {
      if( events | GPIO_IRQ_EDGE_FALL )
      {
	iorq_flag |= SEEN_FALLING;
      }
      if( events | GPIO_IRQ_EDGE_RISE )
      {
	iorq_flag |= SEEN_RISING;
      }

      if( iorq_flag == SEEN_BOTH )
      {
	gpio_set_irq_enabled( GPIO_Z80_IORQ,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
      }
    }
    break;

    default:
      break;
    }
  }
}

void z80_page_run_tests( void )
{
  /*
   * Restart the Z80. This test runs as the computer boots up and runs the
   * ROM code. The sleep is to let the capacitor C27 in the Spectrum charge
   * up and release the RESET line
   */
  gpio_put( GPIO_Z80_RESET, 1 ); sleep_ms( 5 );
  gpio_put( GPIO_Z80_RESET, 0 ); sleep_ms( 650 );

  /* Restart the alarm which defines the duration of the test */
  z80_test_running = true;
  alarm_id_t z80_alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, z80_alarm_callback, NULL, false );
  if( z80_alarm_id < 0 )
    panic("No alarms available in Z80 test");

  while( z80_test_running );

  /*
   * Alarm is done with, this reset isn't necessary but if I change the
   * timing system something else might need to go here
   */
  cancel_alarm( z80_alarm_id );
}


void z80_output(void)
{
  uint8_t line=2;
  for( uint32_t test_index=0; test_index<NUM_Z80_TESTS; test_index++ )
  {      
    draw_str(0, line*8, "                         " );
    draw_str(0, line*8, result_line_txt[test_index] );      
	
    line++;
  }  
}
