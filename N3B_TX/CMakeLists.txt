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
# set(PICO_SDK_PATH "C:/Users/Arjan/Documents/Pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(N3B_TX C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
# n3b_tx.c	main loop and initialisation of the software
# lcd.c		LCD driver stuff; pay attention, X different HW implementations exist
# adf4360.c	The drivers for setting output frequency in the adf4360 chip
# dsp.c		The signal processing stuff, either timedomain or frequency domain
# fix_fft.c	The FFT transformations in fixed point format
# hmi.c		All user interaction, controlling freq, modulation, levels, etc
# monitor.c	A tty shell on a serial interface
add_executable(N3B_TX n3b_main_tx.c lcd.c adf4360.c dsp.c fix_fft.c hmi.c monitor.c i2c_slave.c)

# target_compile_options(N3B_TX PRIVATE -Wall)
pico_set_program_name(N3B_TX "N3B_TX")
pico_set_program_version(N3B_TX "0.1")

# Pull in our pico_stdlib which aggregates commonly used features
target_link_libraries(N3B_TX pico_stdlib)

# Disable uart output, enable usb output
pico_enable_stdio_uart(N3B_TX 0)
pico_enable_stdio_usb(N3B_TX 1)

# Add any user requested libraries
target_link_libraries(N3B_TX
        pico_stdlib
		pico_multicore
		hardware_i2c
		hardware_pwm
        hardware_gpio
        hardware_timer
        hardware_clocks
		hardware_pll
		hardware_adc
		hardware_dma
        )

pico_add_extra_outputs(N3B_TX)

