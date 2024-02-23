/*
 * ZX Diagnostics Firmware, a Raspberry Pi Pico based Spectrum test device
 * Copyright (C) 2024 Derek Fountain
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 * cmake -DCMAKE_BUILD_TYPE=Debug ..
 * make -j10
 * sudo openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program ./zx_diagnostics_pico2.elf verify reset exit"
 * sudo openocd -f interface/picoprobe.cfg -f target/rp2040.cfg
 * gdb-multiarch ./zx_diagnostics_pico2.elf
 *  target remote localhost:3333
 *  load
 *  monitor reset init
 *  continue
 */

#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include <string.h>

#include "test_data.h"
#include "gpios.h"

#include "link_common.h"
#include "picoputer.pio.h"

#define PICO_COMM_TEST_ABUS 0x01020304
#define PICO_COMM_TEST_ROM  0x04030201

static void test_blipper( void )
{
  gpio_put( GPIO_P2_BLIPPER, 1 );
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  gpio_put( GPIO_P2_BLIPPER, 0 );
}

#define ADDR_BUF_SIZE 2048
static uint16_t address_buffer[ADDR_BUF_SIZE*2];

/*
 */
void main( void )
{
  bi_decl(bi_program_description("ZX Spectrum Diagnostics Pico2 Board Binary."));

  /* Let the main Pico get going */
  sleep_ms( 2000 );

  /* Blipper, for the scope */
  gpio_init( GPIO_P2_BLIPPER ); gpio_set_dir( GPIO_P2_BLIPPER, GPIO_OUT ); gpio_put( GPIO_P2_BLIPPER, 0 );

  /* Watch for the incoming cue from the other Pico */
  gpio_init( GPIO_P2_SIGNAL );  gpio_set_dir( GPIO_P2_SIGNAL, GPIO_IN );  gpio_pull_down( GPIO_P2_SIGNAL );

  gpio_init( GPIO_ABUS_A0  );   gpio_set_dir( GPIO_ABUS_A0,  GPIO_IN ); gpio_pull_up( GPIO_ABUS_A0  );
  gpio_init( GPIO_ABUS_A1  );   gpio_set_dir( GPIO_ABUS_A1,  GPIO_IN ); gpio_pull_up( GPIO_ABUS_A1  );
  gpio_init( GPIO_ABUS_A2  );   gpio_set_dir( GPIO_ABUS_A2,  GPIO_IN ); gpio_pull_up( GPIO_ABUS_A2  );
  gpio_init( GPIO_ABUS_A3  );   gpio_set_dir( GPIO_ABUS_A3,  GPIO_IN ); gpio_pull_up( GPIO_ABUS_A3  );
  gpio_init( GPIO_ABUS_A4  );   gpio_set_dir( GPIO_ABUS_A4,  GPIO_IN ); gpio_pull_up( GPIO_ABUS_A4  );
  gpio_init( GPIO_ABUS_A5  );   gpio_set_dir( GPIO_ABUS_A5,  GPIO_IN ); gpio_pull_up( GPIO_ABUS_A5  );
  gpio_init( GPIO_ABUS_A6  );   gpio_set_dir( GPIO_ABUS_A6,  GPIO_IN ); gpio_pull_up( GPIO_ABUS_A6  );
  gpio_init( GPIO_ABUS_A7  );   gpio_set_dir( GPIO_ABUS_A7,  GPIO_IN ); gpio_pull_up( GPIO_ABUS_A7  );
  gpio_init( GPIO_ABUS_A8  );   gpio_set_dir( GPIO_ABUS_A8,  GPIO_IN ); gpio_pull_up( GPIO_ABUS_A8  );
  gpio_init( GPIO_ABUS_A9  );   gpio_set_dir( GPIO_ABUS_A9,  GPIO_IN ); gpio_pull_up( GPIO_ABUS_A9  );
  gpio_init( GPIO_ABUS_A10 );   gpio_set_dir( GPIO_ABUS_A10, GPIO_IN ); gpio_pull_up( GPIO_ABUS_A10 );
  gpio_init( GPIO_ABUS_A11 );   gpio_set_dir( GPIO_ABUS_A11, GPIO_IN ); gpio_pull_up( GPIO_ABUS_A11 );
  gpio_init( GPIO_ABUS_A12 );   gpio_set_dir( GPIO_ABUS_A12, GPIO_IN ); gpio_pull_up( GPIO_ABUS_A12 );
  gpio_init( GPIO_ABUS_A13 );   gpio_set_dir( GPIO_ABUS_A13, GPIO_IN ); gpio_pull_up( GPIO_ABUS_A13 );
  gpio_init( GPIO_ABUS_A14 );   gpio_set_dir( GPIO_ABUS_A14, GPIO_IN ); gpio_pull_up( GPIO_ABUS_A14 );
  gpio_init( GPIO_ABUS_A15 );   gpio_set_dir( GPIO_ABUS_A15, GPIO_IN ); gpio_pull_up( GPIO_ABUS_A15 );

  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  const enum gpio_function linkout_function = GPIO_FUNC_PIO0;
  const PIO                linkout_pio      = pio0;
  const enum gpio_function linkin_function  = GPIO_FUNC_PIO0;
  const PIO                linkin_pio       = pio0;

  /* Outbound link, to Pico1 */
  gpio_set_function(GPIO_P2_LINKOUT, linkout_function);

  int  linkout_sm = pio_claim_unused_sm(linkout_pio, true);
  uint offset     = pio_add_program(linkout_pio, &picoputerlinkout_program);
  picoputerlinkout_program_init(linkout_pio, linkout_sm, offset, GPIO_P2_LINKOUT);

  /* Inbound link, from Pico1 */
  gpio_set_function(GPIO_P2_LINKIN, linkin_function);

  int  linkin_sm  = pio_claim_unused_sm(linkin_pio, true);
       offset     = pio_add_program(linkin_pio, &picoputerlinkin_program);
  picoputerlinkin_program_init(linkin_pio, linkin_sm, offset, GPIO_P2_LINKIN);

  /* Let everything settle before this end starts listening */
  sleep_ms( 1000 );

  uint32_t test_counter = 0;
  while( 1 )
  {
gpio_put( LED_PIN, 0 );
    /* Wait for test type from the other Pico */
    uint32_t test_type = PICO_COMM_TEST_ABUS;
    ui_link_receive_buffer( linkin_pio, linkin_sm, linkout_sm, (uint8_t*)&test_type, sizeof(test_type) );

    switch( test_type )
    {
    case PICO_COMM_TEST_ABUS:
    {
    /* Read the GPIO_P2_SIGNAL, wait for it to go high */
      while( gpio_get( GPIO_P2_SIGNAL ) == 0 );
gpio_put( LED_PIN, 1 );
    
      /*
       * Address bus test, just monitor the address lines and confirm they got low->high and high->low
       * Loop while the first Pico is holding the "test running" signal
       */
//      SEEN_EDGE line_edge[16] =	{ SEEN_NEITHER, SEEN_NEITHER, SEEN_NEITHER, SEEN_NEITHER, 
      //			  SEEN_NEITHER, SEEN_NEITHER, SEEN_NEITHER, SEEN_NEITHER, 
      //			  SEEN_NEITHER, SEEN_NEITHER, SEEN_NEITHER, SEEN_NEITHER, 
      //			  SEEN_NEITHER, SEEN_NEITHER, SEEN_NEITHER, SEEN_NEITHER };
      SEEN_EDGE line_edge[16] =	{ SEEN_BOTH, SEEN_BOTH, SEEN_BOTH, SEEN_BOTH, 
				  SEEN_BOTH, SEEN_BOTH, SEEN_BOTH, SEEN_BOTH, 
				  SEEN_BOTH, SEEN_BOTH, SEEN_BOTH, SEEN_BOTH, 
				  SEEN_BOTH, SEEN_BOTH, SEEN_BOTH, SEEN_BOTH };
//      while( gpio_get( GPIO_P2_SIGNAL ) == 1 )
      //    {
      sleep_ms(100);
      //}

      /* Send response */

      /* Test data starts with a 32 bit number, which is the length of what follows */
// This length is redundant
      uint32_t length = sizeof(line_edge)*sizeof(line_edge[0]) + sizeof(uint32_t);
      ui_link_send_buffer( linkout_pio, linkout_sm, linkin_sm, (uint8_t*)&length, sizeof(length) );

      /* Send buffer load */
      ui_link_send_buffer( linkout_pio, linkout_sm, linkin_sm, (uint8_t*)line_edge, sizeof(line_edge)*sizeof(line_edge[0]) );

      /* Send GPIO state so other Pico can see what lines are stuck, if any */
      uint32_t gpio_state = gpio_get_all();
      ui_link_send_buffer( linkout_pio, linkout_sm, linkin_sm, (uint8_t*)&gpio_state, sizeof(uint32_t) );
    }
    break;

    default:
    {
      /*
       * Unknown test type. No idea what to do. It's probably just a glitch on
       * the line, it sometimes does that at startup. Pause for a moment to let
       * it settle (maybe, probably not actually helpful) then ignore it.
       */
      sleep_ms(1);
    }
    break;

    }

  }

}
