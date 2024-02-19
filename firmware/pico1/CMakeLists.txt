cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico1 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico1
	zx_diagnostics_pico1.c
        font.c	
        sh1106.c
        oled.c
	page_voltages.c
	page_ula.c
	page_z80.c
	page_dbus.c
	page_abus.c
	page_rom.c
)

pico_generate_pio_header(pico1 ${CMAKE_CURRENT_LIST_DIR}/clk_counter.pio)

target_link_libraries(pico1
		      pico_multicore
		      hardware_pio
		      pico_stdlib
		      hardware_i2c
		      hardware_adc)

pico_add_extra_outputs(pico1)
