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


#ifndef _inc_font
#define _inc_font
#include <stdint.h>
typedef struct {
	uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;   /*!< Font height in pixels */
	const uint16_t *data; /*!< Pointer to data font data array */
} FontDef_t;

/** 
 * @brief  String length and height 
 */
typedef struct {
	uint16_t Length;      /*!< String length in units of pixels */
	uint16_t Height;      /*!< String height in units of pixels */
} FONTS_SIZE_t;






/*
 * Format
 * <height>, <width>, <additional spacing per char>, 
 * <first ascii char>, <last ascii char>,
 * <data>
 */

/**
 * @defgroup FONTS_FontVariables
 * @brief    Library font variables
 * @{
 */

/**
 * @brief  7 x 10 pixels font size structure 
 */
extern FontDef_t Font_7x10;

/**
 * @brief  11 x 18 pixels font size structure 
 */
extern FontDef_t Font_11x18;

/**
 * @brief  16 x 26 pixels font size structure 
 */
extern FontDef_t Font_16x26;

/**
 * @}
 */
 
/**
 * @defgroup FONTS_Functions
 * @brief    Library functions
 * @{
 */

/**
 * @brief  Calculates string length and height in units of pixels depending on string and font used
 * @param  *str: String to be checked for length and height
 * @param  *SizeStruct: Pointer to empty @ref FONTS_SIZE_t structure where informations will be saved
 * @param  *Font: Pointer to @ref FontDef_t font used for calculations
 * @retval Pointer to string used for length and height
 */
char* FONTS_GetStringSize(char* str, FONTS_SIZE_t* SizeStruct, FontDef_t* Font);



#endif
