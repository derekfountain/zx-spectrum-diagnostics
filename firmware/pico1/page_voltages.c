/*
 *  -5V voltage divider enters ADC0 which is GPIO26
 * +12V voltage divider enters ADC1 which is GPIO27
 *  +5V voltage divider enters ADC2 which is GPIO28
 */

#include "oled.h"
#include "page.h"

#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"

/* Store last 50 entries so I can work out the averages */
#define AVERAGE_ARRAY_LEN (uint32_t)50
static uint32_t average_index = 0;

static float average_5v[AVERAGE_ARRAY_LEN];
static float average_12v[AVERAGE_ARRAY_LEN];
static float average_min5v[AVERAGE_ARRAY_LEN];

#define NUM_VOLTAGE_TESTS 6
#define WIDTH_OLED_CHARS 32
static uint8_t result_line_txt[NUM_VOLTAGE_TESTS][WIDTH_OLED_CHARS+1];

void voltage_page_init( void )
{
  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(27);
  adc_gpio_init(28);
}

void voltage_page_entry( void )
{
}

/*
 * When the tests have all run, work out the new averages
 */
void voltage_page_exit( void )
{
  float average;

  average = 0.0;
  for( uint32_t av_index=0; av_index<AVERAGE_ARRAY_LEN; av_index++ )
    average += average_5v[av_index];

  snprintf( result_line_txt[3], WIDTH_OLED_CHARS, "AV  +5V: %0.1fV", average / AVERAGE_ARRAY_LEN );


  average = 0.0;
  for( uint32_t av_index=0; av_index<AVERAGE_ARRAY_LEN; av_index++ )
    average += average_12v[av_index];

  snprintf( result_line_txt[4], WIDTH_OLED_CHARS, "AV +12V: %0.1fV", average / AVERAGE_ARRAY_LEN );


  average = 0.0;
  for( uint32_t av_index=0; av_index<AVERAGE_ARRAY_LEN; av_index++ )
    average += average_min5v[av_index];

  snprintf( result_line_txt[5], WIDTH_OLED_CHARS, "AV  -5V: %0.1fV", average / AVERAGE_ARRAY_LEN );


  /* Another set of tests run, move the average circular store index */
  if( ++average_index == AVERAGE_ARRAY_LEN )
    average_index = 0;
}

/*
 * These are all 1K5 except R105 on the 12V supply which is 56K.
 * I measured mine out of circuit before soldering them
 */
#define R101 1498.0
#define R102 1498.0
#define R103 1496.0
#define R104 1496.0
#define R105 5650.0
#define R106 1496.0

/*
 * Some worked examples from my test Spectrum:
 *
 *  +5V on the edge connector is  5.03V
 * +12V on the edge connector is 12.30V
 *  -5V on the edge connector is  1.24V (because it's broken)
 *
 *  5V_DIVIDED on the board's test point is 2.52V, should be 2.515V
 * 12V_DIVIDED on the board's test point is 2.61V, should be 2.577V
 * -5V_DIVIDED on the board's test point is 3.14V, should be 3.79/2 = 1.895V
 *   (so that -5V one is over 500mV off)
 *
 *  +5V ADC value is approx 3210
 * +12V ADC value is approx 3320 (variable, goes to over 3400 sometimes)
 *  -5V ADC value is approx 3995 (variable, goes over 4000 quite frequently)
 *
 *  +5V calculation is (3210 * 0.00081) / (1.0 - 0.5000) =  5.20V, shown value is  5.16V
 * +12V calculation is (3320 * 0.00081) / (1.0 - 0.7906) = 12.84V, shown value is 12.75V
 *  -5V calculation is (3995 * 0.00081) / (1.0 - 0.5001) - 5.00 = 1.473V shown value is 1.40V
 *
 * So +5V is 130mV off, which is mostly in the ADC conversion. +12V is close, but
 * it's a wobbly voltage at the best of times because of the way it's created.
 * On this board, -5V is broken, so I'm not sure what to trust. The voltage divider
 * seems to be putting out a weird value, but it's hard to tell what's a fault.
 *
 * I tried a board with a valid -5V supply and the OLED report said -4.47ish, which
 * is exactly what I'd expect. So I think it's OK.
 */
const float conversion_factor = 3.3f / (1 << 12);          // 0.00081

const float _5v_ratio     = 1.0 - (R103 / (R103 + R104));  // 0.50000 on my test board
const float _12v_ratio    = 1.0 - (R105 / (R105 + R106));  // 0.20935 on my test board
const float _minus5_ratio = 1.0 - (R101 / (R101 + R102));  // 0.49997 on my test board

void voltage_page_test_5v( void )
{
  adc_select_input( 2 );
  float reading = (adc_read() * conversion_factor) / _5v_ratio;
  snprintf( result_line_txt[0], WIDTH_OLED_CHARS, "    +5V: %0.3fV", reading );

  average_5v[average_index] = reading;
}


void voltage_page_test_12v( void )
{
  adc_select_input( 1 );
  float reading = (adc_read() * conversion_factor) / _12v_ratio;
  snprintf( result_line_txt[1], WIDTH_OLED_CHARS, "   +12V: %0.3fV", reading );

  average_12v[average_index] = reading;
}


void voltage_page_test_minus5v( void )
{
  adc_select_input( 0 );
  float reading = ((adc_read() * conversion_factor) / _minus5_ratio) - 5.00;
  // float reading = adc_read();
  snprintf( result_line_txt[2], WIDTH_OLED_CHARS, "    -5V: %0.3fV", reading );

  average_min5v[average_index] = reading;
}


void voltage_output(void)
{
  /*
   * I've only got 6 lines, so there's a bit of crude, hard coded jiggling
   * about here to get a gap between the values and the average values.
   */
  uint8_t line=2;
  for( uint32_t test_index=0; test_index<3; test_index++ )
  {      
    draw_str(0, line*8-2, "                         " );
    draw_str(0, line*8-2, result_line_txt[test_index] );      
	
    line++;
  }

  for( uint32_t test_index=3; test_index<6; test_index++ )
  {      
    draw_str(0, line*8+1, "                         " );
    draw_str(0, line*8+1, result_line_txt[test_index] );      
	
    line++;
  }
}
