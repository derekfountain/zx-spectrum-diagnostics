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

static       int linkin_sm;
static const PIO linkin_pio = pio1;
#define LINKIN_PIN 18

uint32_t num_bytes_received = 0;

void abus_page_init( void )
{
  /* Set up the GPIO which pokes the other Pico */
  gpio_init( GPIO_P1_SIGNAL ); gpio_set_dir( GPIO_P1_SIGNAL, GPIO_OUT ); gpio_put( GPIO_P1_SIGNAL, 0 );

  /* Inbound link, from address bus handling Pico */
  gpio_set_function(LINKIN_PIN, GPIO_FUNC_PIO1);    

  linkin_sm       = pio_claim_unused_sm(linkin_pio, true);
  uint offset     = pio_add_program(linkin_pio, &picoputerlinkin_program);
  picoputerlinkin_program_init(linkin_pio, linkin_sm, offset, LINKIN_PIN);
}

void abus_page_entry( void )
{
gpio_put( GPIO_P1_SIGNAL, 1 );
}

void abus_page_exit( void )
{
gpio_put( GPIO_P1_SIGNAL, 0 );
}

void abus_page_gpios( uint32_t gpio, uint32_t events )
{
}

void abus_page_run_tests( void )
{
  /* Read from PIO input FIFO */
  uint32_t data = 0;
  while( ! picoputerlinkin_get(linkin_pio, linkin_sm, &data) );

  /* Invert what's been received */
  data = data ^ 0xFFFFFFFF;

  /* It arrives from the PIO at the top end of the word, so shift down */
  data >>= 22;
      
  /* Remove stop bit in LSB */
  data >>= 1;
	  
  /* Mask out data, just to be sure */
  data &= 0xff;

//  if( data == 0xDF )
//  {
    num_bytes_received++;
    snprintf( result_line_txt[0], WIDTH_OLED_CHARS, "abus: 0x%08X", /*num_bytes_received,*/ data);
//  }
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
