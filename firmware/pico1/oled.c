#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "gpios.h"
#include "font.h"
#include "sh1106.h"

static const uint8_t *font;

int init_oled( const uint8_t *f )
{  
  i2c_init(i2c_default, 400 * 1000);
  gpio_set_function( GPIO_OLED_SDA, GPIO_FUNC_I2C );
  gpio_set_function( GPIO_OLED_SCK, GPIO_FUNC_I2C );
  gpio_pull_up( GPIO_OLED_SDA );
  gpio_pull_up( GPIO_OLED_SCK );
    
  SH1106_Init();

  if( f )
    font = f;
  else
    font = font_8x5;

  return 0;
}


void draw_pixel( uint32_t x, uint32_t y )
{
  SH1106_DrawPixel(x, y, (SH1106_COLOR_t)1 );
}


void clear_pixel( uint32_t x, uint32_t y )
{
  SH1106_DrawPixel(x, y, (SH1106_COLOR_t)0 );
}


void draw_char(uint32_t x, uint32_t y, uint8_t c )
{
  if(c<font[3]||c>font[4])
    return;
  
  uint32_t parts_per_line=(font[0]>>3)+((font[0]&7)>0);

  for(uint8_t w=0; w<font[1]; ++w)     // width
  {
    // font[0] is the height, font[3] is first char in font

    uint32_t pp=(c-font[3])*font[1]*parts_per_line+w*parts_per_line+5;  
    
    for(uint32_t lp=0; lp<parts_per_line; ++lp)
    {
      uint8_t line=font[pp];
      
      for(int8_t j=0; j<8; ++j, line>>=1)
      {
	if(line & 1)
	{
	  draw_pixel( x+w, y+((lp<<3)+j) );
	}
	else
	{
	  clear_pixel( x+w, y+((lp<<3)+j) );
	}
      }
	
      ++pp;
    }
  }
}


void draw_str( uint32_t x, uint32_t y, uint8_t *str )
{
  uint8_t *next_char = str;

  while( *next_char )
  {
    draw_char( x, y, *next_char++ );
    x += (font[1]+font[2]);
  }

}


void update_screen( void )
{
  SH1106_UpdateScreen();
}


void clear_screen( void )
{
  SH1106_Clear();
}
