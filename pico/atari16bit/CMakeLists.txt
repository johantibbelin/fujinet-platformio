# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(main)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(main
    main.c
)
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/acsi_cmd.pio)
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/dma_out.pio)
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/dma_in.pio)
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/wait_cmd.pio)
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/send_status.pio)
pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/send_irq.pio)

pico_enable_stdio_usb(main 1)
# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main pico_stdlib pico_multicore hardware_spi hardware_dma hardware_pio hardware_irq)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)
