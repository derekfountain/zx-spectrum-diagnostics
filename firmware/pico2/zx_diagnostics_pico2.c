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

#include "gpios.h"

#include "link_common.h"
#include "picoputer.pio.h"

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

#define ADDR_BUF_SIZE 64
static uint8_t address_buffer[256];

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

uint32_t buffer_index = 0;
for(buffer_index=0; buffer_index<ADDR_BUF_SIZE;buffer_index++)
  address_buffer[buffer_index] = buffer_index & 0xFF;

  while( 1 )
  {
      sleep_ms(2000);
    /* Read the GPIO_P2_SIGNAL, wait for it to go high */
//    while( gpio_get( GPIO_P2_SIGNAL ) == 0 );

    /* Loop: Read the address bus, stash value */

    /* Check if GPIO_P2_SIGNAL has switched off */
    /* No? Loop back for another address bus read, yes, exit loop */

    /* Send number of bytes we have in the buffer */

    /* Send buffer load */

    /* Back to top of loop, wait for next test run signal */




//    while (gpio_get( GPIO_P2_SIGNAL ))
//    {

      gpio_put( LED_PIN, 1 );
      sleep_ms(50);
      gpio_put( LED_PIN, 0 );


      test_blipper();

//      const uint32_t data = 0xDF;
      ui_link_send_buffer( linkout_pio, linkout_sm, linkin_sm, address_buffer, ADDR_BUF_SIZE );

//      ui_link_send_byte( linkin_pio, linkout_sm, linkin_sm, data );
//      pio_sm_put_blocking(pio0, linkout_sm, 0x200 | ((data ^ 0xff)<<1));

      test_blipper();
       
//    }


  }

}
