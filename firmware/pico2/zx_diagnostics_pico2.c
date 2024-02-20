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

#define LINKOUT_PIN 22

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
  gpio_put( GPIO_P2_BLIPPER, 0 );
}

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

  /* Outbound link, to IO Pico */
  gpio_set_function(LINKOUT_PIN, GPIO_FUNC_PIO0);

  int  linkout_sm      = pio_claim_unused_sm(pio0, true);
  uint offset     = pio_add_program(pio0, &picoputerlinkout_program);
  picoputerlinkout_program_init(pio0, linkout_sm, offset, LINKOUT_PIN);

  while( 1 )
  {
    /* Read the GPIO_P2_SIGNAL, wait for it to go high */
//    while( gpio_get( GPIO_P2_SIGNAL ) == 0 );

//    while (gpio_get( GPIO_P2_SIGNAL ))
//    {
      sleep_ms(1000);

      gpio_put( LED_PIN, 1 );
      sleep_ms(50);
      gpio_put( LED_PIN, 0 );


      test_blipper();
      const uint32_t data = 0xDF;
      pio_sm_put_blocking(pio0, linkout_sm, 0x200 | ((data ^ 0xff)<<1));
      test_blipper();
       
//    }


  }

}
