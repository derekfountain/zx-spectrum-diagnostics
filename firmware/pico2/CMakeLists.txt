cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico2 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico2
	       zx_diagnostics_pico2.c
	       ../firmware-common/link_common.c
)

target_include_directories(pico2 PRIVATE ../firmware-common)

pico_generate_pio_header(pico2 ../../firmware-common/picoputer.pio)

target_include_directories(pico2 PRIVATE ../firmware-common)

target_link_libraries(pico2
		      pico_stdlib
		      hardware_pio
	              hardware_gpio
)

pico_add_extra_outputs(pico2)
