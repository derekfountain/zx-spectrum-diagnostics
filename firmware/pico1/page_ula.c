/*
 * INT line.
 * This runs once as the test page is entered. Need to do better.
 */

#include "oled.h"
#include "page.h"
#include "gpios.h"

#include "pico/stdlib.h"
#include <stdio.h>

#define INT_TEST_TIME_SECS   5
#define INT_TEST_TIME_SECS_F ((float)(INT_TEST_TIME_SECS_F))
static uint32_t int_counter = 0;

alarm_id_t int_alarm_id = -1;

static bool int_test_running = false;

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
  int_test_running = false;
  return 0;
}

static void ula_page_entry( void )
{
  gpio_put( GPIO_Z80_RESET, 0 );
  gpio_init( GPIO_Z80_INT ); gpio_set_dir( GPIO_Z80_INT, GPIO_IN ); gpio_pull_up( GPIO_Z80_INT );

  gpio_set_irq_enabled( GPIO_Z80_INT, GPIO_IRQ_EDGE_FALL, true );

  int_counter = 0;

  int_test_running = true;
  int_alarm_id = add_alarm_in_ms( INT_TEST_TIME_SECS*1000, alarm_callback, NULL, false );  
}

static void ula_page_exit( void )
{
  if( int_alarm_id != -1 )
    cancel_alarm( int_alarm_id );

  gpio_set_irq_enabled( GPIO_Z80_INT, GPIO_IRQ_EDGE_FALL, false );
  gpio_put( GPIO_Z80_RESET, 1 );  
}

static void ula_page_gpios( uint gpio, uint32_t events )
{
  if( gpio == GPIO_Z80_INT )
  {
    if( int_test_running )
      int_counter++;
  }
}

static void ula_page_test( void )
{
  uint8_t int_line[32];

  if( int_test_running )
    sprintf( int_line, "INT: ????" );
  else
    sprintf( int_line, "INT: %0.2fHz", ((float)(int_counter)) / INT_TEST_TIME_SECS );
  draw_str(0, 0, int_line);

  update_screen();
}

static page_t ula_page = 
{
  ula_page_entry,
  ula_page_exit,
  ula_page_test,
  ula_page_gpios,
  500,
};

page_t *get_ula_page( void )
{
  return &ula_page;
}
