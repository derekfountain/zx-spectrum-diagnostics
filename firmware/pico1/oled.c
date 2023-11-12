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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "font.h"
#include "sh1106.h"


int main()
{  
  stdio_init_all();

  i2c_init(i2c_default, 400 * 1000);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    
  SH1106_Init();

  SH1106_GotoXY(0,0);
  SH1106_Puts("X", &Font_7x10, 1);

  SH1106_GotoXY(120,0);
  SH1106_Puts("X", &Font_7x10, 1);

  SH1106_GotoXY(0,53);
  SH1106_Puts("X", &Font_7x10, 1);

  SH1106_GotoXY(120,53);
  SH1106_Puts("X", &Font_7x10, 1);

  uint16_t x;
  for( x=11; x<118; x++ )
    SH1106_DrawPixel( x, 4, 1);

  SH1106_UpdateScreen();

  while (1)
  {
    sleep_ms(1000);
  }

  return 0;
}
