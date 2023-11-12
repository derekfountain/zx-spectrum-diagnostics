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


#define SH1106_RIGHT_HORIZONTAL_SCROLL              0x26
#define SH1106_LEFT_HORIZONTAL_SCROLL               0x27
#define SH1106_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SH1106_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A
#define SH1106_DEACTIVATE_SCROLL                    0x2E // Stop scroll
#define SH1106_ACTIVATE_SCROLL                      0x2F // Start scroll
#define SH1106_SET_VERTICAL_SCROLL_AREA             0xA3 // Set scroll range

#define SH1106_NORMALDISPLAY       0xA6
#define SH1106_INVERTDISPLAY       0xA7


void SH1106_ScrollRight(uint8_t start_row, uint8_t end_row)
{
  SH1106_WRITECOMMAND (SH1106_RIGHT_HORIZONTAL_SCROLL);  // send 0x26
  SH1106_WRITECOMMAND (0x00);  // send dummy
  SH1106_WRITECOMMAND(start_row);  // start page address
  SH1106_WRITECOMMAND(0X00);  // time interval 5 frames
  SH1106_WRITECOMMAND(end_row);  // end page address
  SH1106_WRITECOMMAND(0X00);
  SH1106_WRITECOMMAND(0XFF);
  SH1106_WRITECOMMAND (SH1106_ACTIVATE_SCROLL); // start scroll
}


void SH1106_ScrollLeft(uint8_t start_row, uint8_t end_row)
{
  SH1106_WRITECOMMAND (SH1106_LEFT_HORIZONTAL_SCROLL);  // send 0x26
  SH1106_WRITECOMMAND (0x00);  // send dummy
  SH1106_WRITECOMMAND(start_row);  // start page address
  SH1106_WRITECOMMAND(0X00);  // time interval 5 frames
  SH1106_WRITECOMMAND(end_row);  // end page address
  SH1106_WRITECOMMAND(0X00);
  SH1106_WRITECOMMAND(0XFF);
  SH1106_WRITECOMMAND (SH1106_ACTIVATE_SCROLL); // start scroll
}


void SH1106_Scrolldiagright(uint8_t start_row, uint8_t end_row)
{
  SH1106_WRITECOMMAND(SH1106_SET_VERTICAL_SCROLL_AREA);  // sect the area
  SH1106_WRITECOMMAND (0x00);   // write dummy
  SH1106_WRITECOMMAND(SH1106_HEIGHT);

  SH1106_WRITECOMMAND(SH1106_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
  SH1106_WRITECOMMAND (0x00);
  SH1106_WRITECOMMAND(start_row);
  SH1106_WRITECOMMAND(0X00);
  SH1106_WRITECOMMAND(end_row);
  SH1106_WRITECOMMAND (0x01);
  SH1106_WRITECOMMAND (SH1106_ACTIVATE_SCROLL);
}


void SH1106_Scrolldiagleft(uint8_t start_row, uint8_t end_row)
{
  SH1106_WRITECOMMAND(SH1106_SET_VERTICAL_SCROLL_AREA);  // sect the area
  SH1106_WRITECOMMAND (0x00);   // write dummy
  SH1106_WRITECOMMAND(SH1106_HEIGHT);

  SH1106_WRITECOMMAND(SH1106_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
  SH1106_WRITECOMMAND (0x00);
  SH1106_WRITECOMMAND(start_row);
  SH1106_WRITECOMMAND(0X00);
  SH1106_WRITECOMMAND(end_row);
  SH1106_WRITECOMMAND (0x01);
  SH1106_WRITECOMMAND (SH1106_ACTIVATE_SCROLL);
}


void SH1106_Stopscroll(void)
{
	SH1106_WRITECOMMAND(SH1106_DEACTIVATE_SCROLL);
}



void SH1106_InvertDisplay (int i)
{
  if (i) SH1106_WRITECOMMAND (SH1106_INVERTDISPLAY);

  else SH1106_WRITECOMMAND (SH1106_NORMALDISPLAY);

}


void SH1106_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color)
{

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    for(int16_t j=0; j<h; j++, y++)
    {
        for(int16_t i=0; i<w; i++)
        {
            if(i & 7)
            {
               byte <<= 1;
            }
            else
            {
               byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }
            if(byte & 0x80) SH1106_DrawPixel(x+i, y, color);
        }
    }
}


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
	

	SH1106_WRITECOMMAND(SH1106_DEACTIVATE_SCROLL);

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

void SH1106_ToggleInvert(void) {
	uint16_t i;
	
	/* Toggle invert */
	SH1106.Inverted = !SH1106.Inverted;
	
	/* Do memory toggle */
	for (i = 0; i < sizeof(SH1106_Buffer); i++) {
		SH1106_Buffer[i] = ~SH1106_Buffer[i];
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

void SH1106_GotoXY(uint16_t x, uint16_t y) {
	/* Set write pointers */
	SH1106.CurrentX = x;
	SH1106.CurrentY = y;
}

char SH1106_Putc(char ch, FontDef_t* Font, SH1106_COLOR_t color) {
	uint32_t i, b, j;
	
	/* Check available space in LCD */
	if (
		SH1106_WIDTH <= (SH1106.CurrentX + Font->FontWidth) ||
		SH1106_HEIGHT <= (SH1106.CurrentY + Font->FontHeight)
	) {
		/* Error */
		return 0;
	}
	
	/* Go through font */
	for (i = 0; i < Font->FontHeight; i++) {
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				SH1106_DrawPixel(SH1106.CurrentX + j, (SH1106.CurrentY + i), (SH1106_COLOR_t) color);
			} else {
				SH1106_DrawPixel(SH1106.CurrentX + j, (SH1106.CurrentY + i), (SH1106_COLOR_t)!color);
			}
		}
	}
	
	/* Increase pointer */
	SH1106.CurrentX += Font->FontWidth;
	
	/* Return character written */
	return ch;
}

char SH1106_Puts(char* str, FontDef_t* Font, SH1106_COLOR_t color) {
	/* Write characters */
	while (*str) {
		/* Write character by character */
		if (SH1106_Putc(*str, Font, color) != *str) {
			/* Return error */
			return *str;
		}
		
		/* Increase string pointer */
		str++;
	}
	
	/* Everything OK, zero should be returned */
	return *str;
}
 
void SH1106_Clear (void)
{
	SH1106_Fill (0);
    SH1106_UpdateScreen();
}
void SH1106_ON(void) {
	SH1106_WRITECOMMAND(0x8D);  
	SH1106_WRITECOMMAND(0x14);  
	SH1106_WRITECOMMAND(0xAF);  
}
void SH1106_OFF(void) {
	SH1106_WRITECOMMAND(0x8D);  
	SH1106_WRITECOMMAND(0x10);
	SH1106_WRITECOMMAND(0xAE);  
}

void sh1106_I2C_WriteMulti(uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) {
uint8_t dt[256];
dt[0] = reg;
uint8_t i;
for(i = 0; i < count; i++)
dt[i+1] = data[i];
//HAL_I2C_Master_Transmit(&hi2c1, address, dt, count+1, 10);
	i2c_write_blocking(i2c_default, address, dt, count+1, 10);

}


void sh1106_I2C_Write(uint8_t address, uint8_t reg, uint8_t data) {
	uint8_t dt[2];
	dt[0] = reg;
	dt[1] = data;
	//HAL_I2C_Master_Transmit(&hi2c1, address, dt, 2, 10);
  i2c_write_blocking(i2c_default, address, dt, 2, 10);

}
