# Generated Cmake Pico project file
#
# After changing this file, empty the build folder and execute from there:  
#     cmake -G "NMake Makefiles" ..
#

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
# set(PICO_SDK_PATH "/Users/barisdinc/PICO/pico-sdk")
# set(PICO_SDK_PATH "../../pico-sdk/")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(N3B_RX C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
# n3b_rx_main.c	main loop and initialisation of the software
#//TODO:complete the list of files
add_executable(N3B_RX n3b_rx_main.c hardware_init.c debug_interface.c 
				display_ili9341.c
				user_interface.c
				adf4360.c
				lib/assert.c
				lib/ili9341/ili9341.c
				)

target_compile_options(N3B_RX PRIVATE -Wall)
pico_set_program_name(N3B_RX "N3B_RX")
pico_set_program_version(N3B_RX "0.0.1")

# Pull in our pico_stdlib which aggregates commonly used features
target_link_libraries(N3B_RX pico_stdlib)

# Disable uart output, enable usb output
pico_enable_stdio_uart(N3B_RX 0)
pico_enable_stdio_usb(N3B_RX 1)

# Add any user requested libraries
target_link_libraries(N3B_RX
        pico_stdlib
		pico_multicore
		hardware_i2c
		hardware_spi
		hardware_pwm
        hardware_gpio
        hardware_timer
        hardware_clocks
		hardware_pll
		hardware_adc
		hardware_dma
        )

pico_add_extra_outputs(N3B_RX)

