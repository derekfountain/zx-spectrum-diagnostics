/* GPIOs for Pico1, the one with the data bus and most of the Z80 signals */

#ifndef __GPIOS_H
#define GPIOS_H

/* These need to match the hardware design */

/* Test signal to scope */
#define GPIO_P1_BLIPPER   9

/* I2C to screen */
#define GPIO_OLED_SDA    16
#define GPIO_OLED_SCK    17

/* Pico1 to Pico2 signal */
#define GPIO_P1_SIGNAL    8

/* Click on user button */
#define GPIO_INPUT1      21

/* Pico2 to Pico1 serial data link pin, input to Pico1 */
#define GPIO_P2_LINKOUT  18

/* Pico1 to Pico2 serial data link pin, input to Pico2 */
#define GPIO_P2_LINKIN   19

/* Output, holds Z80 in reset */
#define GPIO_Z80_RESET   22

#define GPIO_Z80_MREQ    10
#define GPIO_Z80_IORQ    20
#define GPIO_Z80_RD      11
#define GPIO_Z80_WR      12
#define GPIO_Z80_M1      13
#define GPIO_Z80_CLK     14
#define GPIO_Z80_INT     15

#define GPIO_DBUS_D0      0
#define GPIO_DBUS_D1      1
#define GPIO_DBUS_D2      2
#define GPIO_DBUS_D3      3
#define GPIO_DBUS_D4      4
#define GPIO_DBUS_D5      5
#define GPIO_DBUS_D6      6
#define GPIO_DBUS_D7      7

#endif
