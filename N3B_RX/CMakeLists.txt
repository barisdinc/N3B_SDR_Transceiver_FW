# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Werror -fdata-sections -ffunction-sections)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)


project(N3B_rx)
pico_sdk_init()

file(GLOB U8G2_SRCS
     "external/u8g2/csrc/*.c"
)

include_directories(include external/u8g2/csrc/)

add_library(
    u8g2
    ${U8G2_SRCS}
)

target_compile_options(u8g2 PRIVATE -Wno-unused-function -Wno-unused-variable)

if(PICO_BOARD STREQUAL "pico")

  #main application
  add_executable(N3B_rx
      ssd1306.c
      main_rx.cpp
      vco.c
      nco.cpp
      rx.cpp
      rx_dsp.cpp
      fft.cpp
      fft_filter.cpp
      cic_corrections.cpp
      ui.cpp
      utils.cpp
      ili934x.cpp
      waterfall.cpp
      button.cpp
      cat.cpp
      usb_descriptors.c
      usb_audio_device.c
      ring_buffer_lib.c
      adf4360.cpp
  )

  pico_generate_pio_header(N3B_rx ${CMAKE_CURRENT_LIST_DIR}/nco.pio)
  pico_generate_pio_header(N3B_rx ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)
  pico_add_extra_outputs(N3B_rx)
  # enable usb output, disable uart output
  pico_enable_stdio_usb(N3B_rx 1)
  pico_enable_stdio_uart(N3B_rx 0)
  target_include_directories(N3B_rx PRIVATE ${CMAKE_CURRENT_LIST_DIR})
  target_link_libraries(N3B_rx PRIVATE pico_stdlib
                                       pico_multicore
                                       hardware_clocks
                                       hardware_flash
                                       hardware_pio
                                       hardware_adc
                                       hardware_pwm
                                       hardware_dma 
                                       hardware_i2c
                                       hardware_spi
                                       tinyusb_device
                                       tinyusb_board
                                       u8g2)
  target_compile_definitions(N3B_rx PUBLIC PICO_XOSC_STARTUP_DELAY_MULTIPLIER=128)

  # #battery check utility
  # project(battery_check)
  # add_executable(battery_check
  #     battery_check.cpp
  # )
  # pico_add_extra_outputs(battery_check)
  # pico_enable_stdio_usb(battery_check 1)
  # pico_enable_stdio_uart(battery_check 0)
  # target_link_libraries(battery_check pico_stdlib hardware_adc)

elseif(PICO_BOARD STREQUAL "pico2")


  # if(PICO_PLATFORM STREQUAL "rp2350-riscv")

  #   #main application
  #   add_executable(N3B_rx-riscv
  #     ssd1306.c
  #     main_rx.cpp
  #     vco.c
  #     rx.cpp
  #     rx_dsp.cpp
  #     fft.cpp
  #     fft_filter.cpp
  #     cic_corrections.cpp
  #     ui.cpp
  #     utils.cpp
  #     ili934x.cpp
  #     waterfall.cpp
  #     button.cpp
  #     cat.cpp
  #     usb_descriptors.c
  #     usb_audio_device.c
  #     ring_buffer_lib.c
  #   )
  #   # pico_generate_pio_header(N3B_rx-riscv ${CMAKE_CURRENT_LIST_DIR}/nco.pio)
  #   pico_generate_pio_header(N3B_rx-riscv ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)
  #   pico_add_extra_outputs(N3B_rx-riscv)
  #   # enable usb output, disable uart output
  #   pico_enable_stdio_usb(N3B_rx-riscv 1)
  #   pico_enable_stdio_uart(N3B_rx-riscv 0)
  #   target_include_directories(N3B_rx-riscv PRIVATE ${CMAKE_CURRENT_LIST_DIR})
  #   target_link_libraries(N3Brx-riscv PRIVATE pico_stdlib
  #                                        pico_multicore
  #                                        hardware_clocks
  #                                        hardware_flash
  #                                        hardware_pio
  #                                        hardware_adc
  #                                        hardware_pwm
  #                                        hardware_dma 
  #                                        hardware_i2c
  #                                        hardware_spi
  #                                        tinyusb_device
  #                                        tinyusb_board
  #                                        u8g2)
  #   target_compile_definitions(N3B_rx-riscv PUBLIC PICO_XOSC_STARTUP_DELAY_MULTIPLIER=128)

  #   #battery check utility
  #   project(battery_check_pico2-riscv)
  #   add_executable(battery_check_pico2-riscv
  #       battery_check.cpp
  #   )
  #   pico_add_extra_outputs(battery_check_pico2-riscv)
  #   pico_enable_stdio_usb(battery_check_pico2-riscv 1)
  #   pico_enable_stdio_uart(battery_check_pico2-riscv 0)
  #   target_link_libraries(battery_check_pico2-riscv pico_stdlib hardware_adc)

  # else()

  #   #main application
  #   add_executable(N3B_rx2
  #     ssd1306.c
  #     main_rx.cpp
  #     vco.c
  #     rx.cpp
  #     rx_dsp.cpp
  #     fft.cpp
  #     fft_filter.cpp
  #     cic_corrections.cpp
  #     ui.cpp
  #     utils.cpp
  #     ili934x.cpp
  #     waterfall.cpp
  #     button.cpp
  #     cat.cpp
  #     usb_descriptors.c
  #     usb_audio_device.c
  #     ring_buffer_lib.c
  #   )
  #   # pico_generate_pio_header(N3B_rx2 ${CMAKE_CURRENT_LIST_DIR}/nco.pio)
  #   pico_generate_pio_header(N3B_rx2 ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)
  #   pico_add_extra_outputs(N3B_rx2)
  #   pico_enable_stdio_usb(N3B_rx2 1)
  #   pico_enable_stdio_uart(N3B_rx2 0)
  #   target_include_directories(N3B_rx2 PRIVATE ${CMAKE_CURRENT_LIST_DIR})
  #   target_link_libraries(N3B_rx2 PRIVATE pico_stdlib
  #                                        pico_multicore
  #                                        hardware_clocks
  #                                        hardware_flash
  #                                        hardware_pio
  #                                        hardware_adc
  #                                        hardware_pwm
  #                                        hardware_dma 
  #                                        hardware_i2c
  #                                        hardware_spi
  #                                        tinyusb_device
  #                                        tinyusb_board
  #                                        u8g2)
  #   target_compile_definitions(N3B_rx2 PUBLIC PICO_XOSC_STARTUP_DELAY_MULTIPLIER=128)

  #   #battery check utility
  #   project(battery_check_pico2)
  #   add_executable(battery_check_pico2
  #       battery_check.cpp
  #   )
  #   pico_add_extra_outputs(battery_check_pico2)
  #   pico_enable_stdio_usb(battery_check_pico2 1)
  #   pico_enable_stdio_uart(battery_check_pico2 0)
  #   target_link_libraries(battery_check_pico2 pico_stdlib hardware_adc)

  # endif()

endif()
