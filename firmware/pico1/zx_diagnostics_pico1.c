#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "oled.h"

void main( void )
{
  bi_decl(bi_program_description("ZX Spectrum Diagnostics Pico1 Board Binary."));

  stdio_init_all();

  /* Initialise OLED screen with default font */
  init_oled( NULL );
  
  draw_str(4,   0,  "Spectrum Diagnostics");
  draw_char(0,   56, 'X');
  draw_char(123, 56, 'X');

  uint16_t x;
  for( x=0; x<128; x++ )
    draw_pixel( x, 10 );

  update_screen();

  while (1)
  {
    sleep_ms(1000);
  }

}
