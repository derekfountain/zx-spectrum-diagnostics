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

#ifndef SH1106_H
#define SH1106_H 

#include "font.h"
/* I2C address */
#ifndef SH1106_I2C_ADDR
#define SH1106_I2C_ADDR         0x3c
#endif

/* SH1106 settings */
/* SH1106 width in pixels */
#ifndef SH1106_WIDTH
#define SH1106_WIDTH            128
#endif
/* SH1106 LCD height in pixels */
#ifndef SH1106_HEIGHT
#define SH1106_HEIGHT           64
#endif

/**
 * @brief  SH1106 color enumeration
 */
typedef enum {
	SH1106_COLOR_BLACK = 0x00, /*!< Black color, no pixel */
	SH1106_COLOR_WHITE = 0x01  /*!< Pixel is set. Color depends on LCD */
} SH1106_COLOR_t;


/**
 * @brief  Initializes SH1106 LCD
 * @param  None
 * @retval Initialization status:
 *           - 0: LCD was not detected on I2C port
 *           - > 0: LCD initialized OK and ready to use
 */
uint8_t SH1106_Init(void);

/** 
 * @brief  Updates buffer from internal RAM to LCD
 * @note   This function must be called each time you do some changes to LCD, to update buffer from RAM to LCD
 * @param  None
 * @retval None
 */
void SH1106_UpdateScreen(void);

/**
 * @brief  Toggles pixels invertion inside internal RAM
 * @note   @ref SH1106_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  None
 * @retval None
 */
void SH1106_ToggleInvert(void);

/** 
 * @brief  Fills entire LCD with desired color
 * @note   @ref SH1106_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  Color: Color to be used for screen fill. This parameter can be a value of @ref SH1106_COLOR_t enumeration
 * @retval None
 */
void SH1106_Fill(SH1106_COLOR_t Color);

/**
 * @brief  Draws pixel at desired location
 * @note   @ref SH1106_UpdateScreen() must called after that in order to see updated LCD screen
 * @param  x: X location. This parameter can be a value between 0 and SH1106_WIDTH - 1
 * @param  y: Y location. This parameter can be a value between 0 and SH1106_HEIGHT - 1
 * @param  color: Color to be used for screen fill. This parameter can be a value of @ref SH1106_COLOR_t enumeration
 * @retval None
 */
void SH1106_DrawPixel(uint16_t x, uint16_t y, SH1106_COLOR_t color);

/**
 * @brief  Sets cursor pointer to desired location for strings
 * @param  x: X location. This parameter can be a value between 0 and SH1106_WIDTH - 1
 * @param  y: Y location. This parameter can be a value between 0 and SH1106_HEIGHT - 1
 * @retval None
 */
void SH1106_GotoXY(uint16_t x, uint16_t y);

/**
 * @brief  Puts character to internal RAM
 * @note   @ref SH1106_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  ch: Character to be written
 * @param  *Font: Pointer to @ref FontDef_t structure with used font
 * @param  color: Color used for drawing. This parameter can be a value of @ref SH1106_COLOR_t enumeration
 * @retval Character written
 */
char SH1106_Putc(char ch, FontDef_t* Font, SH1106_COLOR_t color);

/**
 * @brief  Puts string to internal RAM
 * @note   @ref SH1106_UpdateScreen() must be called after that in order to see updated LCD screen
 * @param  *str: String to be written
 * @param  *Font: Pointer to @ref FontDef_t structure with used font
 * @param  color: Color used for drawing. This parameter can be a value of @ref SH1106_COLOR_t enumeration
 * @retval Zero on success or character value when function failed
 */
char SH1106_Puts(char* str, FontDef_t* Font, SH1106_COLOR_t color);


/**
 * @brief  Initializes SH1106 LCD
 * @param  None
 * @retval Initialization status:
 *           - 0: LCD was not detected on I2C port
 *           - > 0: LCD initialized OK and ready to use
 */
void sh1106_I2C_Init();

/**
 * @brief  Writes single byte to slave
 * @param  *I2Cx: I2C used
 * @param  address: 7 bit slave address, left aligned, bits 7:1 are used, LSB bit is not used
 * @param  reg: register to write to
 * @param  data: data to be written
 * @retval None
 */
void sh1106_I2C_Write(uint8_t address, uint8_t reg, uint8_t data);

/**
 * @brief  Writes multi bytes to slave
 * @param  *I2Cx: I2C used
 * @param  address: 7 bit slave address, left aligned, bits 7:1 are used, LSB bit is not used
 * @param  reg: register to write to
 * @param  *data: pointer to data array to write it to slave
 * @param  count: how many bytes will be written
 * @retval None
 */
void sh1106_I2C_WriteMulti(uint8_t address, uint8_t reg, uint8_t *data, uint16_t count);

/**
 * @brief  Draws the Bitmap
 * @param  X:  X location to start the Drawing
 * @param  Y:  Y location to start the Drawing
 * @param  *bitmap : Pointer to the bitmap
 * @param  W : width of the image
 * @param  H : Height of the image
 * @param  color : 1-> white/blue, 0-> black
 */
void SH1106_DrawBitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color);

// scroll the screen for fixed rows

void SH1106_ScrollRight(uint8_t start_row, uint8_t end_row);


void SH1106_ScrollLeft(uint8_t start_row, uint8_t end_row);


void SH1106_Scrolldiagright(uint8_t start_row, uint8_t end_row);


void SH1106_Scrolldiagleft(uint8_t start_row, uint8_t end_row);


void SH1106_Stopscroll(void);


// inverts the display i = 1->inverted, i = 0->normal

void SH1106_InvertDisplay (int i);

// clear the display

void SH1106_Clear (void);


#endif
