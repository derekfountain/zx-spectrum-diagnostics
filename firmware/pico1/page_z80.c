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
#define TEST_TIME_SECS   3
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))

typedef enum
{
  SEEN_NEITHER = 0x00,
  SEEN_RISING  = 0x01,
  SEEN_FALLING = 0x02,
  SEEN_BOTH    = 0x03,
}
SEEN_EDGE;

static SEEN_EDGE m1_flag   = SEEN_NEITHER;
static SEEN_EDGE rd_flag   = SEEN_NEITHER;
static SEEN_EDGE wr_flag   = SEEN_NEITHER;
static SEEN_EDGE mreq_flag = SEEN_NEITHER;

alarm_id_t z80_alarm_id = -1;

static bool z80_test_running = false;

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
}

void z80_page_entry( void )
{
  m1_flag   = SEEN_NEITHER;
  rd_flag   = SEEN_NEITHER;
  wr_flag   = SEEN_NEITHER;
  mreq_flag = SEEN_NEITHER;

  gpio_set_irq_enabled( GPIO_Z80_M1,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_Z80_RD,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_Z80_WR,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
  gpio_set_irq_enabled( GPIO_Z80_MREQ, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true );
}

void z80_page_exit( void )
{
  gpio_set_irq_enabled( GPIO_Z80_M1,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_Z80_RD,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_Z80_WR,   GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
  gpio_set_irq_enabled( GPIO_Z80_MREQ, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false );
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
  gpio_put( GPIO_Z80_RESET, 1 ); sleep_ms( 650 );
  gpio_put( GPIO_Z80_RESET, 0 ); sleep_ms( 650 );

  /* Restart the alarm which defines the duration of the test */
  z80_test_running = true;
  z80_alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, z80_alarm_callback, NULL, false );

  while( z80_test_running );

  /*
   * Alarm is done with, this reset isn't necessary but if I change the
   * timing system something else might need to go here
   */
  cancel_alarm( z80_alarm_id );
  z80_alarm_id = -1;
}


void z80_page_test_m1( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t result_line[32];

  sprintf( result_line, "  M1: %s", m1_flag == SEEN_BOTH ? "     OK" : "Missing" );
  strncpy( result_txt, result_line, result_txt_max_len );
}

void z80_page_test_rd( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t result_line[32];

  sprintf( result_line, "  RD: %s", rd_flag == SEEN_BOTH ? "Running" : "Inactive" );
  strncpy( result_txt, result_line, result_txt_max_len );
}

void z80_page_test_wr( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t result_line[32];

  sprintf( result_line, "  WR: %s", wr_flag == SEEN_BOTH ? "Running" : "Inactive" );
  strncpy( result_txt, result_line, result_txt_max_len );
}

void z80_page_test_mreq( uint8_t *result_txt, uint32_t result_txt_max_len )
{
  uint8_t result_line[32];

  sprintf( result_line, "MREQ: %s", mreq_flag == SEEN_BOTH ? "Running" : "Inactive" );
  strncpy( result_txt, result_line, result_txt_max_len );
}
