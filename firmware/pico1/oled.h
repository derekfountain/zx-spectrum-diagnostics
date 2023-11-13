#ifndef __OLED_H
#define OLED_H

#include "font.h"

int init_oled( uint8_t *f );
void update_screen( void );
void clear_screen( void );
void draw_char( uint32_t x, uint32_t y, uint8_t c );
void draw_str( uint32_t x, uint32_t y, uint8_t *str );
void draw_pixel( uint32_t x, uint32_t y );
void clear_pixel( uint32_t x, uint32_t y );

#endif
