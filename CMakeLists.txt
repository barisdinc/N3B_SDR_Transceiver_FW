# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

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

project(N3B_RX C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
# N3B.c		main loop and initialisation of the software
# adf4306.c	The drivers for setting output frequency and phase in the adf4306 chip
# dsp.c		The signal processing stuff, either timedomain or frequency domain
# fix_fft.c	The FFT transformations in fixed point format
# hmi.c		All user interaction, controlling freq, modulation, levels, etc
# monitor.c	A tty shell on a serial interface
add_executable(N3B_RX n3b_rx_main.c dsp.c fix_fft.c hmi.c monitor.c ili9341.c gfx.c adf4360.c)

target_compile_options(N3B_RX PRIVATE -Wall)
pico_set_program_name(N3B_RX "N3B_RX")
pico_set_program_version(N3B_RX "0.2")

# Pull in our pico_stdlib which aggregates commonly used features
target_link_libraries(N3B_RX pico_stdlib)

# Disable uart output, enable usb output
pico_enable_stdio_uart(N3B_RX 0)
pico_enable_stdio_usb(N3B_RX 1)

# Add any user requested libraries
target_link_libraries(N3B_RX
        pico_stdlib
		pico_multicore
		hardware_spi
		hardware_i2c
		hardware_pwm
        hardware_gpio
        hardware_timer
        hardware_clocks
		hardware_pll
		hardware_adc
		hardware_dma
        )

pico_add_extra_outputs(N3B_RX)

