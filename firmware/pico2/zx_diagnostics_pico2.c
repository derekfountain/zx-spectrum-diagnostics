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

#include "gpios.h"

#include "link_common.h"
#include "picoputer.pio.h"

#define PICO_COMM_TEST_ABUS 0x01
#define PICO_COMM_TEST_ROM  0x02

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

  sleep_ms( 500 );

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


  uint32_t test_counter = 0;
  while( 1 )
  {
    /* Read the GPIO_P2_SIGNAL, wait for it to go high */
    while( gpio_get( GPIO_P2_SIGNAL ) == 0 );

    gpio_put( LED_PIN, 1 );

    /* Wait for test type from the other Pico */
    uint32_t test_type;
    ui_link_receive_buffer( linkin_pio, linkin_sm, linkout_sm, (uint8_t*)&test_type, sizeof(test_type) );

    switch( test_type )
    {
    case PICO_COMM_TEST_ABUS:
    {
      /* Clear buffer */
      uint32_t buffer_index = 0;
      for(buffer_index=0; buffer_index<ADDR_BUF_SIZE;buffer_index++)
	address_buffer[buffer_index] = 0;

      buffer_index = 0;

      /* Loop while the first Pico is holding the "test running" signal */
      while( gpio_get( GPIO_P2_SIGNAL ) )
      {
	/* Wait for a memory request to start */
	if( gpio_get( GPIO_Z80_MREQ ) == 0 )
	{

	  /* Wait for a read to start, as opposed to a write */
	  if( gpio_get( GPIO_Z80_RD ) == 0 )
	  {
	    /* OK, a Z80 memory read has begun, the address is already on the address bus */
	    uint16_t address_bus = gpio_get_all() & 0xFFFF;

	    /*
	     * FIXME This collects the addresses the Z80 puts on the address bus.
	     * Or at least, those that appear on the address bus. It shows a restarting
	     * pattern, like this:
(gdb) x/1024xh address_buffer
0x20000674 <address_buffer+608>:        0x0000  0x0000  0x0001  0x0002  0x0003  0x0004  0x0005  0x0006
0x20000684 <address_buffer+624>:        0x0007  0x11cb  0x11cc  0x11cd  0x11ce  0x11cf  0x0000  0x0000
0x20000694 <address_buffer+640>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x200006a4 <address_buffer+656>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x200006b4 <address_buffer+672>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x200006c4 <address_buffer+688>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x200006d4 <address_buffer+704>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x200006e4 <address_buffer+720>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x200006f4 <address_buffer+736>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x20000704 <address_buffer+752>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x20000714 <address_buffer+768>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x20000724 <address_buffer+784>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x20000734 <address_buffer+800>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x20000744 <address_buffer+816>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x20000754 <address_buffer+832>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x20000764 <address_buffer+848>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x20000774 <address_buffer+864>:        0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x20000784 <address_buffer+880>:        0x0001  0x0002  0x0003  0x0004  0x0005  0x0006  0x0007  0x11cb
0x20000794 <address_buffer+896>:        0x11cc  0x11cd  0x11ce  0x11cf  0x11d0  0x11d1  0x11d2  0x11d3
0x200007a4 <address_buffer+912>:        0x11d4  0x11d5  0x11d6  0x11d7  0x11d8  0x11d9  0x11da  0x11db
0x200007b4 <address_buffer+928>:        0x11dc  0x11dd  0x11de  0x11df  0x11e0  0x11e1  0x11dc  0x11dd
0x200007c4 <address_buffer+944>:        0x11de  0x11df  0x11e0  0x11e1  0x11dc  0x11dd  0x11de  0x11df
0x200007d4 <address_buffer+960>:        0x11e0  0x11e1  0x11dc  0x11dd  0x11de  0x11df  0x11e0  0x11e1
0x200007e4 <address_buffer+976>:        0x11dc  0x11dd  0x11de  0x11df  0x11e0  0x11e1  0x11dc  0x11dd
0x200007f4 <address_buffer+992>:        0x11de  0x11df  0x11e0  0x11e1  0x11dc  0x11dd  0x11de  0x11df
0x20000804 <address_buffer+1008>:       0x11e0  0x11e1  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000
0x20000814 <address_buffer+1024>:       0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0000  0x0001
0x20000824 <address_buffer+1040>:       0x0002  0x0003  0x0004  0x0005  0x0006  0x0007  0x11cb  0x11cc
0x20000834 <address_buffer+1056>:       0x11cd  0x11ce  0x11cf  0x11d0  0x11d1  0x11d2  0x11d3  0x11d4
0x20000844 <address_buffer+1072>:       0x11d5  0x11d6  0x11d7  0x11d8  0x11d9  0x11da  0x11db  0x11dc
0x20000854 <address_buffer+1088>:       0x11dd  0x11de  0x11df  0x11e0  0x11e1  0x11dc  0x11dd  0x11de

So it runs addresses 0, 1, 2, etc, up to 0x11cf in the frist case, then starts again. I saw exactly this
when I was doing the ROM emulator. Lots of zeroes appear, then it starts again, getting a bit further.
A few more zeroes, then it starts again. Eventually, after 3 or 4 iterations, it keeps going and the 
ROM program runs. I don't know why the Z80/Spectrum does this, but it appears consistent.
	     */

	    /* Read the address bus, stash value while there's room in the buffer */
	    address_buffer[buffer_index++] = address_bus;
	    if( buffer_index == ADDR_BUF_SIZE )
	      break;

	    /* Wait for the Z80 complete the memory request */
	    while( gpio_get( GPIO_Z80_MREQ ) == 0 );
	  }
	  
	}


      }
    }
    break;

    default:
      break;
    }

    gpio_put( LED_PIN, 0 );

    /* Send response, whatever that is */
    /* Test data starts with a 32 bit number, which is the length of what follows */
    uint32_t length = sizeof(test_counter);
    ui_link_send_buffer( linkout_pio, linkout_sm, linkin_sm, (uint8_t*)&length, sizeof(length) );

    /* Send number of bytes we have in the buffer */

    /* Send buffer load */
    memcpy( address_buffer, (uint8_t*)&test_counter, sizeof(test_counter) );
    ui_link_send_buffer( linkout_pio, linkout_sm, linkin_sm, (uint8_t*)address_buffer, length );
       
    /* Back to top of loop, wait for next test run signal */
    test_counter++;
  }

}
