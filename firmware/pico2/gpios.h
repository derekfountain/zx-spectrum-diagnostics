/* GPIOs for Pico2, the one with the address bus */

#ifndef __GPIOS_H
#define GPIOS_H

/* These need to match the hardware design */

/* Test signal to scope */
#define GPIO_P2_BLIPPER   19

#define GPIO_ABUS_A0       0
#define GPIO_ABUS_A1       1
#define GPIO_ABUS_A2       2
#define GPIO_ABUS_A3       3
#define GPIO_ABUS_A4       4
#define GPIO_ABUS_A5       5
#define GPIO_ABUS_A6       6
#define GPIO_ABUS_A7       7
#define GPIO_ABUS_A8       8
#define GPIO_ABUS_A9       9
#define GPIO_ABUS_A10     10
#define GPIO_ABUS_A11     11
#define GPIO_ABUS_A12     12
#define GPIO_ABUS_A13     13
#define GPIO_ABUS_A14     14
#define GPIO_ABUS_A15     15

#define GPIO_P2_SIGNAL    20

/* Pico2 to Pico1 serial data link pin, output from Pico2 */
#define GPIO_P2_LINKOUT   22

/* Pico2 to Pico1 serial data link pin, input to Pico2 */
#define GPIO_P2_LINKIN    21

#endif
