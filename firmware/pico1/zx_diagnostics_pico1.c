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
 * sudo openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program ./zx_diagnostics_pico1.elf verify reset exit"
 * sudo openocd -f interface/picoprobe.cfg -f target/rp2040.cfg
 * gdb-multiarch ./zx_diagnostics_pico1.elf
 *  target remote localhost:3333
 *  load
 *  monitor reset init
 *  continue
 */

#include <string.h>

#include "pico/platform.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"

#include "page.h"
#include "oled.h"
#include "gpios.h"

#include "page_voltages.h"
#include "page_ula.h"
#include "page_z80.h"
#include "page_dbus.h"
#include "page_abus.h"
#include "page_rom.h"

#include "picoputer.pio.h"

static uint8_t input1_pressed = 0;

/* Set of pages, press button 1 to cycle these */
typedef enum
{
  VOLTAGE_PAGE = 0,
  ULA_PAGE,
  Z80_PAGE,
  DBUS_PAGE,
  ABUS_PAGE,
  ROM_PAGE,

  LAST_PAGE = ROM_PAGE
}
PAGE;

PAGE current_page;

/* All the tests the unit currently runs */
typedef enum
{
  TEST_VOLTAGE_5V     = 0,
  TEST_VOLTAGE_12V,
  TEST_VOLTAGE_MIN5V,
  TEST_VOLTAGE_AV5V,
  TEST_VOLTAGE_AV12V,
  TEST_VOLTAGE_AVMIN5V,

  TEST_ULA_INT,
  TEST_ULA_CLK,
  TEST_ULA_CCLK,

  TEST_Z80_M1,
  TEST_Z80_RD,
  TEST_Z80_WR,
  TEST_Z80_MREQ,

  TEST_DBUS_DBUS,

  TEST_ABUS_ABUS,

  TEST_ROM_SEQ,

  NUM_TESTS,
}
TEST_INDEX;

typedef enum
{
  NEEDS_RUNNING,
  RESULT_READY,
}
SHOW_RESULT_FLAG;

/* Inbound link, from second pico which does the address bus stuff */
static       int                linkin_sm;
static const PIO                linkin_pio      = pio1;
static const enum gpio_function linkin_function = GPIO_FUNC_PIO1;
static       uint               linkin_offset;

/* Outbound link, to second pico which does the address bus stuff */
static       int                linkout_sm;
static const PIO                linkout_pio      = pio1;
static const enum gpio_function linkout_function = GPIO_FUNC_PIO1;
static       uint               linkout_offset;

/* Paging, for the user interface */
typedef struct
{
  PAGE              page;
  uint8_t          *gui_title;
  void            (*init_func)(void);                       // Initialise
  void            (*gpio_func)( uint32_t, uint32_t );       // A GPIO line has changed state
  void            (*display_func)( void );                  // Display your output now
  SHOW_RESULT_FLAG  show_result;
}
DISPLAY_PAGE;

DISPLAY_PAGE page[] =
{
  { VOLTAGE_PAGE, "VOLTAGES",    voltage_page_init, NULL,               voltage_output, NEEDS_RUNNING },
  { ULA_PAGE,     "ULA SIGNALS", ula_page_init,     ula_page_gpios,     ula_output,     NEEDS_RUNNING },
  { Z80_PAGE,     "Z80 SIGNALS", z80_page_init,     z80_page_gpios,     z80_output,     NEEDS_RUNNING },
  { DBUS_PAGE,    "DATA BUS",    dbus_page_init,    dbus_page_gpios,    dbus_output,    NEEDS_RUNNING },
  { ABUS_PAGE,    "ADDRESS BUS", abus_page_init,    abus_page_gpios,    abus_output,    NEEDS_RUNNING },
  { ROM_PAGE,     "ROM",         rom_page_init,     rom_page_gpios,     rom_output,     NEEDS_RUNNING },
};
#define NUM_PAGES (sizeof(page) / sizeof(DISPLAY_PAGE))


/* From the timer_lowlevel.c example */
static uint64_t get_time_us( void )
{
  uint32_t lo = timer_hw->timelr;
  uint32_t hi = timer_hw->timehr;
  return ((uint64_t)hi << 32u) | lo;
}

/*
 * GPIO event handler callback. This manages the user input button, plus
 * any GPIO handling the tests require
 */
static uint64_t debounce_timestamp_us = 0;
void gpios_callback( uint gpio, uint32_t events ) 
{
  /*
   * Switch event, set to interrupt on rising edge, so this is a click up.
   */
  if( gpio == GPIO_INPUT1 )
  {
#define DEBOUNCE_USECS 100000
    /* Debounce pause, the switch is a bit noisy */
    if( (get_time_us() - debounce_timestamp_us) < DEBOUNCE_USECS )
    {
      /* If last switch action was very recently, assume it's a bounce and ignore it */
      debounce_timestamp_us = get_time_us();
    }
    else
    {
      /* Debounced, take action - just set a flag */
      if( gpio == GPIO_INPUT1 )
      {
	input1_pressed = 1;
      }

      /* Note this point as when we last actioned a switch */
      debounce_timestamp_us = get_time_us();
    }
  }
  else
  {
    /*
     * A GPIO (which isn't the user interface switches) has changed.
     * Pass the details into the code which is running the tests.
     */
    if( page[current_page].gpio_func != NULL )
    {
      (page[current_page].gpio_func)( gpio, events );
    }
  }
}


/*
 * Core 1 runs the tests
 */
static void core1_main( void )
{
  /*
   * This core handles the test GPIO handling, and therefore has to handle the
   * user interface buttons as well (because only one GPIO IRQ handler is allowed)
   *
   * Initialise the switch button GPIOs
   */
  gpio_init( GPIO_INPUT1 ); gpio_set_dir( GPIO_INPUT1, GPIO_IN ); gpio_pull_up( GPIO_INPUT1 );

  /* Put GPIOs callback in place so the user buttons work */
  gpio_set_irq_enabled_with_callback( GPIO_INPUT1, GPIO_IRQ_EDGE_RISE, true, &gpios_callback );

  /* Run all the pages' initialisation functions */
  for( uint32_t page_index = 0; page_index < NUM_PAGES; page_index++ )
  {
    if( page[page_index].init_func != NULL )
    {
      (page[page_index].init_func)();
    }
  }

  while( 1 )
  {
    switch( current_page )
    {
    case VOLTAGE_PAGE:
    {
      /***
       *     __   __     _  _                        
       *     \ \ / /___ | || |_  __ _  __ _  ___  ___
       *      \ V // _ \| ||  _|/ _` |/ _` |/ -_)(_-<
       *       \_/ \___/|_| \__|\__,_|\__, |\___|/__/
       *                              |___/          
       */
      
      /* Initialise the voltage tests */
      voltage_page_entry();

      /* Run the 5V rail test and populate the result line for the display */
      voltage_page_test_5v();
      
      /* Run the 12V rail test and populate the result line for the display */
      voltage_page_test_12v();
      
      /* Run the -5V rail test and populate the result line for the display */
      voltage_page_test_minus5v();
      
      /* Tear down voltage tests */
      voltage_page_exit();

      page[VOLTAGE_PAGE].show_result = RESULT_READY;
    }
    break;

    case ULA_PAGE:
    {
      /***
       *      _   _  _       _   
       *     | | | || |     /_\  
       *     | |_| || |__  / _ \ 
       *      \___/ |____|/_/ \_\
       *                         
       */
      
      /* Initialise the ULA tests */
      ula_page_entry();

      /* The ULA tests are all run in one function */
      ula_page_run_tests();
    
      /* Tear down ULA tests */
      ula_page_exit();

      page[ULA_PAGE].show_result = RESULT_READY;
    }
    break;

    case Z80_PAGE:
    {
      /***
       *      ____ ___   __  
       *     |_  /( _ ) /  \ 
       *      / / / _ \| () |
       *     /___|\___/ \__/ 
       *                     
       */

      /* Initialise the Z80 tests */
      z80_page_entry();

      /* Run the Z80 tests and populate the result lines for the display */
      z80_page_run_tests();

      /* Tear down Z80 tests */
      z80_page_exit();

      page[Z80_PAGE].show_result = RESULT_READY;
    }
    break;

    case DBUS_PAGE:
    {
      /***
       *      ___         _           ___           
       *     |   \  __ _ | |_  __ _  | _ ) _  _  ___
       *     | |) |/ _` ||  _|/ _` | | _ \| || |(_-<
       *     |___/ \__,_| \__|\__,_| |___/ \_,_|/__/
       *                                            
       */

      /* Initialise the data bus tests */
      dbus_page_entry();

      /* Run the data bus tests and populate the result lines for the display */
      dbus_page_run_tests();

      /* Tear down data bus tests */
      dbus_page_exit();

      page[DBUS_PAGE].show_result = RESULT_READY;
    }
    break;

    case ABUS_PAGE:
    {
      /***
       *        _       _     _                     ___           
       *       /_\   __| | __| | _ _  ___  ___ ___ | _ ) _  _  ___
       *      / _ \ / _` |/ _` || '_|/ -_)(_-<(_-< | _ \| || |(_-<
       *     /_/ \_\\__,_|\__,_||_|  \___|/__//__/ |___/ \_,_|/__/
       *                                                          
       */

      /* Initialise the address bus tests */
      abus_page_entry();

      /* Run the address bus tests and populate the result lines for the display */
      abus_page_run_tests( linkin_pio, linkout_pio, linkin_sm, linkout_sm );

      /* Tear down address bus tests */
      abus_page_exit();

      page[ABUS_PAGE].show_result = RESULT_READY;
    }
    break;

    case ROM_PAGE:
    {
      /***
       *      ___   ___   __  __ 
       *     | _ \ / _ \ |  \/  |
       *     |   /| (_) || |\/| |
       *     |_|_\ \___/ |_|  |_|
       *                         
       */

      /* Initialise the ROM tests */
      rom_page_entry();
      
      /* Run the ROM sequential bytes test and populate the result lines for the display */
      rom_page_run_seq_test( linkin_pio, linkout_pio, linkin_sm, linkout_sm );

      /* Tear down ROM tests */
      rom_page_exit();

      page[ROM_PAGE].show_result = RESULT_READY;
    }
    break;

    default:
    break;
    }
  }
}


/*
 * Core 0 runs the user interface/screen
 */
void main( void )
{
  bi_decl(bi_program_description("ZX Spectrum Diagnostics Pico1 Board Binary."));

  sleep_ms( 1000 );

  /* Inbound link, from address bus handling Pico2. The GPIO is labelled from Pico2's view */
  gpio_set_function(GPIO_P2_LINKOUT, linkin_function);    

  linkin_sm       = pio_claim_unused_sm(linkin_pio, true);
  linkin_offset   = pio_add_program(linkin_pio, &picoputerlinkin_program);
  picoputerlinkin_program_init(linkin_pio, linkin_sm, linkin_offset, GPIO_P2_LINKOUT);

  /* Outbound link, to address bus handling Pico2. The GPIO is labelled from Pico2's view */
  gpio_set_function(GPIO_P2_LINKIN, linkout_function);    

  linkout_sm      = pio_claim_unused_sm(linkout_pio, true);
  linkout_offset  = pio_add_program(linkout_pio, &picoputerlinkout_program);
  picoputerlinkout_program_init(linkout_pio, linkout_sm, linkout_offset, GPIO_P2_LINKIN);

  /* Initialise OLED screen with default font */
  init_oled( NULL );

  /* Set up the GPIO which pokes the other Pico */
  gpio_init( GPIO_P1_SIGNAL ); gpio_set_dir( GPIO_P1_SIGNAL, GPIO_OUT ); gpio_put( GPIO_P1_SIGNAL, 0 );

  /* Blipper, for the scope */
  gpio_init( GPIO_P1_BLIPPER ); gpio_set_dir( GPIO_P1_BLIPPER, GPIO_OUT ); gpio_put( GPIO_P1_BLIPPER, 0 );

  /* Start with the Z80 running */
  gpio_init( GPIO_Z80_RESET ); gpio_set_dir( GPIO_Z80_RESET, GPIO_OUT ); gpio_put( GPIO_Z80_RESET, 0 );

  /* Init complete, run 2nd core code */
  multicore_launch_core1( core1_main ); 

  /* Start with voltages page */
  current_page = VOLTAGE_PAGE;

  /* Main loop just loops over the result text lines displaying them */
  while( 1 )
  {
    /*
     * If the user button has been pressed, as indicated by the GPIO callback routine,
     * move to the next page. The new page to show is flagged as needing to be run so
     * stale test result data it's holding isn't shown.
     */
    if( input1_pressed )
    {
      if( current_page++ == LAST_PAGE )
	current_page = VOLTAGE_PAGE;

      page[current_page].show_result = NEEDS_RUNNING;

      input1_pressed = 0;

      clear_screen();
    }

    draw_str(0, 0, page[current_page].gui_title );

    if( page[current_page].show_result == RESULT_READY )
    {
      /* Call the module's display function, it prints its own results */
      (page[current_page].display_func)();
    }
    else
    {
      draw_str(0, 2*8, "  Tests running...   " );
    }
    update_screen();    

    sleep_ms( 100 );
  }

}
