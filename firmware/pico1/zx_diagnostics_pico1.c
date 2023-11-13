#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "page.h"
#include "oled.h"

#include "page_voltages.h"

static page_t *current_page;

void main( void )
{
  bi_decl(bi_program_description("ZX Spectrum Diagnostics Pico1 Board Binary."));

  stdio_init_all();

  /* Initialise OLED screen with default font */
  init_oled( NULL );
  
  current_page = get_voltage_page();
  (current_page->entry_fn)();

  while( 1 )
  {
    // This can't take too long. If it turns out that tests need to
    // run a long time, this will have to be done on the second core
    //
    (current_page->tests_fn)();

    for( uint32_t i=0; i < current_page->repeat_pause_ms; i++ )
    {
      sleep_ms( 1 );

      // Sample button, if pressed change current_page to next
      // Run exit function
      // Update pointer
      // Run entry function
      break;
    }
  }

}
