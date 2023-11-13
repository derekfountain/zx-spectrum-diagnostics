/*

MIT License

Copyright (c) 2021 David Schramm
Copyright (c) 2023 Derek Fountain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

https://github.com/derekfountain/pico-sh1106-oled
*/

#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <pico/binary_info.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sh1106.h"
#include "font.h"
#include "sh1106.h"

/* Write command */
#define SH1106_WRITECOMMAND(command)      sh1106_I2C_Write(SH1106_I2C_ADDR, 0x00, (command))
/* Write data */
#define SH1106_WRITEDATA(data)            sh1106_I2C_Write(SH1106_I2C_ADDR, 0x40, (data))
/* Absolute value */
#define ABS(x)   ((x) > 0 ? (x) : -(x))

/* SH1106 data buffer */
static uint8_t SH1106_Buffer[SH1106_WIDTH * SH1106_HEIGHT / 8];

/* Private SH1106 structure */
typedef struct {
	uint16_t CurrentX;
	uint16_t CurrentY;
	uint8_t Inverted;
	uint8_t Initialized;
} SH1106_t;

/* Private variable */
static SH1106_t SH1106;

uint8_t SH1106_Init(void) {
	/* A little delay */
	uint32_t p = 2500;
	while(p>0)
		p--;
	
	/* Init LCD */
	SH1106_WRITECOMMAND(0xAE); //display off
	SH1106_WRITECOMMAND(0x20); //Set Memory Addressing Mode   
	SH1106_WRITECOMMAND(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	SH1106_WRITECOMMAND(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	SH1106_WRITECOMMAND(0xC8); //Set COM Output Scan Direction
	SH1106_WRITECOMMAND(0x00); //---set low column address
	SH1106_WRITECOMMAND(0x10); //---set high column address
	SH1106_WRITECOMMAND(0x40); //--set start line address
	SH1106_WRITECOMMAND(0x81); //--set contrast control register
	SH1106_WRITECOMMAND(0xFF);
	SH1106_WRITECOMMAND(0xA1); //--set segment re-map 0 to 127
	SH1106_WRITECOMMAND(0xA6); //--set normal display
	SH1106_WRITECOMMAND(0xA8); //--set multiplex ratio(1 to 64)
	SH1106_WRITECOMMAND(0x3F); //
	SH1106_WRITECOMMAND(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	SH1106_WRITECOMMAND(0xD3); //-set display offset
	SH1106_WRITECOMMAND(0x00); //-not offset
	SH1106_WRITECOMMAND(0xD5); //--set display clock divide ratio/oscillator frequency
	SH1106_WRITECOMMAND(0xF0); //--set divide ratio
	SH1106_WRITECOMMAND(0xD9); //--set pre-charge period
	SH1106_WRITECOMMAND(0x22); //
	SH1106_WRITECOMMAND(0xDA); //--set com pins hardware configuration
	SH1106_WRITECOMMAND(0x12);
	SH1106_WRITECOMMAND(0xDB); //--set vcomh
	SH1106_WRITECOMMAND(0x20); //0x20,0.77xVcc
	SH1106_WRITECOMMAND(0x8D); //--set DC-DC enable
	SH1106_WRITECOMMAND(0x14); //
	SH1106_WRITECOMMAND(0xAF); //--turn on SH1106 panel
	

	/* Clear screen */
	SH1106_Fill(SH1106_COLOR_BLACK);
	
	/* Update screen */
	SH1106_UpdateScreen();
	
	/* Set default values */
	SH1106.CurrentX = 0;
	SH1106.CurrentY = 0;
	
	/* Initialized OK */
	SH1106.Initialized = 1;
	
	/* Return OK */
	return 1;
}

void SH1106_UpdateScreen(void) {
	uint8_t m;
	
	for (m = 0; m < 8; m++) {
		SH1106_WRITECOMMAND(0xB0 + m);
		SH1106_WRITECOMMAND(0x00);
		SH1106_WRITECOMMAND(0x10);
		
		/* Write multi data */
		sh1106_I2C_WriteMulti(SH1106_I2C_ADDR, 0x40, &SH1106_Buffer[SH1106_WIDTH * m], SH1106_WIDTH);
	}
}

void SH1106_Fill(SH1106_COLOR_t color) {
	/* Set memory */
	memset(SH1106_Buffer, (color == SH1106_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SH1106_Buffer));
}

void SH1106_DrawPixel(uint16_t x, uint16_t y, SH1106_COLOR_t color) {
	if (
		x >= SH1106_WIDTH ||
		y >= SH1106_HEIGHT
	) {
		/* Error */
		return;
	}
	
	/* Check if pixels are inverted */
	if (SH1106.Inverted) {
		color = (SH1106_COLOR_t)!color;
	}
	
	/* Set color */
	if (color == SH1106_COLOR_WHITE) {
		SH1106_Buffer[x + (y / 8) * SH1106_WIDTH] |= 1 << (y % 8);
	} else {
		SH1106_Buffer[x + (y / 8) * SH1106_WIDTH] &= ~(1 << (y % 8));
	}
}

void SH1106_Clear (void)
{
    SH1106_Fill (0);
    SH1106_UpdateScreen();
}

void sh1106_I2C_WriteMulti(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
  uint8_t dt[256];
  dt[0] = reg;
  uint8_t i;
  for(i = 0; i < count; i++)
    dt[i+1] = data[i];
  i2c_write_blocking(i2c_default, address, dt, count+1, 10);
}


void sh1106_I2C_Write(uint8_t address, uint8_t reg, uint8_t data) {
  uint8_t dt[2];
  dt[0] = reg;
  dt[1] = data;
  i2c_write_blocking(i2c_default, address, dt, 2, 10);
}


