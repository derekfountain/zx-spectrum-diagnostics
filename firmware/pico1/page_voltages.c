/*
 *  -5V voltage divider enters ADC0 which is GPIO26
 * +12V voltage divider enters ADC1 which is GPIO27
 *  +5V voltage divider enters ADC2 which is GPIO28
 */

#include "oled.h"
#include "page.h"

#include <stdio.h>
#include "hardware/adc.h"

static void voltage_page_entry( void )
{
  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(27);
  adc_gpio_init(28);
}

static void voltage_page_exit( void )
{
}

static void voltage_page_test( void )
{
  const float conversion_factor = 3.3f / (1 << 12);

  uint8_t _5v_line[32];
  uint8_t _minus_5v_line[32];
  uint8_t _12v_line[32];

  adc_select_input( 2 );
  sprintf( _5v_line,       " +5V: %0.3fV", adc_read() * conversion_factor );
  draw_str(0,  0, _5v_line);

  adc_select_input( 1 );
  sprintf( _12v_line,      "+12V: %0.3fV", adc_read() * conversion_factor );
  draw_str(0,  8, _12v_line);

  adc_select_input( 0 );
  sprintf( _minus_5v_line, " -5V: %0.3fV", adc_read() * conversion_factor );
  draw_str(0, 16, _minus_5v_line);

  update_screen();
}

static page_t voltage_page = 
{
  voltage_page_entry,
  voltage_page_exit,
  voltage_page_test,
  1000,
};

page_t *get_voltage_page( void )
{
  return &voltage_page;
}
