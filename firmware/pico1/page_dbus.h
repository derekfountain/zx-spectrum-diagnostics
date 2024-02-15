#ifndef __PAGE_DBUS_H
#define __PAGE_DBUS_H

#include "page.h"

void dbus_page_init( void );
void dbus_page_entry( void );
void dbus_page_gpios( uint32_t gpio, uint32_t events );
void dbus_page_run_tests( void );
void dbus_output(void);
void dbus_page_exit( void );

#endif
