/*
 * Address bus tests
 */

#include "oled.h"
#include "page.h"
#include "gpios.h"

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#include "link_common.h"
#include "picoputer.pio.h"

/* Long enough to let the Spectrum boot and run a decent part of the ROM */
#define TEST_TIME_SECS   2
#define TEST_TIME_SECS_F ((float)(TEST_TIME_SECS))

#define NUM_ABUS_TEST_RESULT_LINES 1
#define WIDTH_OLED_CHARS 32
static uint8_t result_line_txt[NUM_ABUS_TEST_RESULT_LINES][WIDTH_OLED_CHARS+1];

#define PICO_COMM_TEST_ABUS 0x01
#define PICO_COMM_TEST_ROM  0x02

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
  abus_test_running = false;

  /*
   * Reboot the Spectrum.
   */
  gpio_put( GPIO_Z80_RESET, 1 ); sleep_ms( 5 );
  gpio_put( GPIO_Z80_RESET, 0 );

  /* Flag the other Pico, which monitors the address bus */
  gpio_put( GPIO_P1_SIGNAL, 1 );

  /* Tell the other Pico which test to run */
  uint32_t test_type = PICO_COMM_TEST_ABUS;
  ui_link_send_buffer( linkout_pio, linkout_sm, linkin_sm, (uint8_t*)&test_type, sizeof(test_type) );

  /* Start the alarm which defines the duration of the test */
  abus_test_running = true;
  alarm_id_t abus_alarm_id = add_alarm_in_ms( TEST_TIME_SECS*1000, abus_alarm_callback, NULL, false );
  if( abus_alarm_id < 0 )
    panic("No alarms available in ABUS test");

  while( abus_test_running );

  /* Remove flag to stop the other Pico collecting address data */
  gpio_put( GPIO_P1_SIGNAL, 0 );

  /*
   * Alarm is done with, this reset isn't necessary but if I change the
   * timing system something else might need to go here
   */
  cancel_alarm( abus_alarm_id );

  /* Wait for 4 byte length value from the other Pico */
  uint32_t buffer_length;
  ui_link_receive_buffer( linkin_pio, linkin_sm, linkout_sm, (uint8_t*)&buffer_length, sizeof(buffer_length) );

  /* Receive that many bytes from the other Pico */
  ui_link_receive_buffer( linkin_pio, linkin_sm, linkout_sm, address_buffer, buffer_length );

  /* Analyse the bytes and populate the result line */
  uint32_t test_counter;
  memcpy( (uint8_t*)&test_counter, address_buffer, sizeof(test_counter) );
  

  /* Show the result line */
  snprintf( result_line_txt[0], WIDTH_OLED_CHARS, "%u %02X %02X %02X %02X", test_counter,
	    address_buffer[0],
	    address_buffer[1],
	    address_buffer[2],
	    address_buffer[3] );
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
