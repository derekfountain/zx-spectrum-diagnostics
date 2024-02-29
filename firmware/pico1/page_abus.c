/*
 * Address bus tests
 */

#include "oled.h"
#include "page.h"
#include "gpios.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#include "test_data.h"
#include "link_common.h"
#include "picoputer.pio.h"

/* Long enough to let the Spectrum boot and run a decent part of the ROM */
#define TEST_TIME_SECS   2
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))

#define NUM_ABUS_TEST_RESULT_LINES 6
#define WIDTH_OLED_CHARS 32
static uint8_t result_line_txt[NUM_ABUS_TEST_RESULT_LINES][WIDTH_OLED_CHARS+1];

#define PICO_COMM_TEST_ABUS 0x01020304
#define PICO_COMM_TEST_ROM  0x04030201

/* Inbound link, from second pico which does the address bus stuff */
static       int                linkin_sm;
static const PIO                linkin_pio      = pio1;
static const enum gpio_function linkin_function = GPIO_FUNC_PIO1;

/* Outbound link, to second pico which does the address bus stuff */
static       int                linkout_sm;
static const PIO                linkout_pio      = pio1;
static const enum gpio_function linkout_function = GPIO_FUNC_PIO1;

static uint32_t num_bytes_received = 0;

static bool abus_test_running = false;

void abus_page_init( void )
{
  /* Set up the GPIO which pokes the other Pico */
  gpio_init( GPIO_P1_SIGNAL ); gpio_set_dir( GPIO_P1_SIGNAL, GPIO_OUT ); gpio_put( GPIO_P1_SIGNAL, 0 );

  /* Inbound link, from address bus handling Pico2. The GPIO is labelled from Pico2's view */
  gpio_set_function(GPIO_P2_LINKOUT, linkin_function);    

  linkin_sm       = pio_claim_unused_sm(linkin_pio, true);
  uint offset     = pio_add_program(linkin_pio, &picoputerlinkin_program);
  picoputerlinkin_program_init(linkin_pio, linkin_sm, offset, GPIO_P2_LINKOUT);

  /* Outbound link, to address bus handling Pico2. The GPIO is labelled from Pico2's view */
  gpio_set_function(GPIO_P2_LINKIN, linkout_function);    

  linkout_sm      = pio_claim_unused_sm(linkout_pio, true);
  offset          = pio_add_program(linkout_pio, &picoputerlinkout_program);
  picoputerlinkout_program_init(linkout_pio, linkout_sm, offset, GPIO_P2_LINKIN);
}

void abus_page_entry( void )
{
}

void abus_page_exit( void )
{
}

void abus_page_gpios( uint32_t gpio, uint32_t events )
{
}

static int64_t __time_critical_func(abus_alarm_callback)(alarm_id_t id, void *user_data)
{
  abus_test_running = false;
  return 0;
}

#define ADDR_BUF_SIZE 2048
static uint8_t address_buffer[ADDR_BUF_SIZE*2];

void abus_page_run_tests( void )
{
#if 0
  /* LED can be useful for this one */
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
#endif

  abus_test_running = false;

  /*
   * Reboot the Spectrum.
   */
  gpio_put( GPIO_Z80_RESET, 1 ); sleep_ms( 5 );
  gpio_put( GPIO_Z80_RESET, 0 );

  /* Tell the other Pico which test to run */
  uint32_t test_type = PICO_COMM_TEST_ABUS;
  ui_link_send_buffer( linkout_pio, linkout_sm, linkin_sm, (uint8_t*)&test_type, sizeof(test_type) );

  /* Flag the other Pico, which monitors the address bus */
  gpio_put( GPIO_P1_SIGNAL, 1 );
  // gpio_put(LED_PIN, 1);

  /* Start the alarm which defines the duration of the test */
  abus_test_running = true;
  alarm_id_t abus_alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, abus_alarm_callback, NULL, false );
  if( abus_alarm_id < 0 )
    panic("No alarms available in ABUS test");

  while( abus_test_running );

  /* Remove flag to stop the other Pico collecting address data */
  gpio_put( GPIO_P1_SIGNAL, 0 );
  //gpio_put(LED_PIN, 0);

  /*
   * Alarm is done with, this reset isn't necessary but if I change the
   * timing system something else might need to go here
   */
  cancel_alarm( abus_alarm_id );

  /* Wait for 16 byte results array from the other Pico */
  SEEN_EDGE line_edge[16];
  ui_link_receive_buffer( linkin_pio, linkin_sm, linkout_sm, (uint8_t*)line_edge, sizeof(line_edge[0])*16 );

  /* Now receive the raw state of the Pico2 GPIOs, so I can report what's stuck, if anything */
  uint32_t gpio_state;
  ui_link_receive_buffer( linkin_pio, linkin_sm, linkout_sm, (uint8_t*)&gpio_state, sizeof(gpio_state) );

  /* Show the result line */
  snprintf( result_line_txt[0], WIDTH_OLED_CHARS, "111111          " );
  snprintf( result_line_txt[1], WIDTH_OLED_CHARS, "5432109876543210" );
  snprintf( result_line_txt[2], WIDTH_OLED_CHARS, "----------------" );
  if( (line_edge[15] == SEEN_BOTH) &&
      (line_edge[14] == SEEN_BOTH) &&
      (line_edge[13] == SEEN_BOTH) &&
      (line_edge[12] == SEEN_BOTH) &&
      (line_edge[11] == SEEN_BOTH) &&
      (line_edge[10] == SEEN_BOTH) &&
      (line_edge[ 9] == SEEN_BOTH) &&
      (line_edge[ 8] == SEEN_BOTH) &&
      (line_edge[ 7] == SEEN_BOTH) &&
      (line_edge[ 6] == SEEN_BOTH) &&
      (line_edge[ 5] == SEEN_BOTH) &&
      (line_edge[ 4] == SEEN_BOTH) &&
      (line_edge[ 3] == SEEN_BOTH) &&
      (line_edge[ 2] == SEEN_BOTH) &&
      (line_edge[ 1] == SEEN_BOTH) &&
      (line_edge[ 0] == SEEN_BOTH) )
  {
    /* This would be the norm: transitions low to high and high to low have all been seen */
    snprintf( result_line_txt[3], WIDTH_OLED_CHARS, "YYYYYYYYYYYYYYYY");
    result_line_txt[4][0] = '\0';
    snprintf( result_line_txt[5], WIDTH_OLED_CHARS, "All active");
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
    snprintf( result_line_txt[3], WIDTH_OLED_CHARS, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
	      line_edge[15] == SEEN_BOTH ? '.' : gpio_state & (1<<15) ? 'H' : 'L',
	      line_edge[14] == SEEN_BOTH ? '.' : gpio_state & (1<<14) ? 'H' : 'L',
	      line_edge[13] == SEEN_BOTH ? '.' : gpio_state & (1<<13) ? 'H' : 'L',
	      line_edge[12] == SEEN_BOTH ? '.' : gpio_state & (1<<12) ? 'H' : 'L',
	      line_edge[11] == SEEN_BOTH ? '.' : gpio_state & (1<<11) ? 'H' : 'L',
	      line_edge[10] == SEEN_BOTH ? '.' : gpio_state & (1<<10) ? 'H' : 'L',
	      line_edge[ 9] == SEEN_BOTH ? '.' : gpio_state & (1<< 9) ? 'H' : 'L',
	      line_edge[ 8] == SEEN_BOTH ? '.' : gpio_state & (1<< 8) ? 'H' : 'L',
	      line_edge[ 7] == SEEN_BOTH ? '.' : gpio_state & (1<< 7) ? 'H' : 'L',
	      line_edge[ 6] == SEEN_BOTH ? '.' : gpio_state & (1<< 6) ? 'H' : 'L',
	      line_edge[ 5] == SEEN_BOTH ? '.' : gpio_state & (1<< 5) ? 'H' : 'L',
	      line_edge[ 4] == SEEN_BOTH ? '.' : gpio_state & (1<< 4) ? 'H' : 'L',
	      line_edge[ 3] == SEEN_BOTH ? '.' : gpio_state & (1<< 3) ? 'H' : 'L',
	      line_edge[ 2] == SEEN_BOTH ? '.' : gpio_state & (1<< 2) ? 'H' : 'L',
	      line_edge[ 1] == SEEN_BOTH ? '.' : gpio_state & (1<< 1) ? 'H' : 'L',
	      line_edge[ 0] == SEEN_BOTH ? '.' : gpio_state & (1<< 0) ? 'H' : 'L'
      );
    result_line_txt[4][0] = '\0';
    snprintf( result_line_txt[5], WIDTH_OLED_CHARS, "Stuck lines");
  }

  /*
   * Repeat test a regular intervals, but don't do it too fast in case the comms
   * goes a bit funny. No reason it should, but let's not tempt it
   */
  sleep_ms(1000);
}


void abus_output(void)
{
  uint8_t line=2;
  for( uint32_t test_index=0; test_index<NUM_ABUS_TEST_RESULT_LINES; test_index++ )
  {      
    draw_str(0, line*8, "                         " );
    draw_str(0, line*8, result_line_txt[test_index] );      
	
    line++;
  }  
}
