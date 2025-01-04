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


cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_TOOLCHAIN_PATH "${USERHOME}/.pico-sdk/toolchain/${toolchainVersion}")


set(CMAKE_BUILD_TYPE Debug)

set(PROJECT_NAME strip_driver)
set(OUTPUT_NAME strip_driver)

# Choose the variety of Pico. This is for future expansion; right now only Pico W is supported.
set(PICO_BOARD pico_w CACHE STRING "Board type")
# LMAO pico_2w doesn't exist yet
# set(PICO_BOARD pico_2w CACHE STRING "Board type")

# Point to the repo's submodules for the PicoSDK and FreeRTOS
set(PICO_SDK_PATH "../pico-sdk/")
set(FREERTOS_KERNEL_PATH "FreeRTOS-Kernel")

# Figure out board support pakcage based on PICO_BOARD
if(${PICO_BOARD} STREQUAL "pico_w")
    set(FREERTOS_KERNEL_PORT_RELATIVE_PATH "portable/ThirdParty/GCC/RP2040")
else()
    message(WARNING "Unrecognized PICO_BOARD value: ${PICO_BOARD}. Only 'pico_w' is supported at present.")
endif()

set(FREERTOS_PORT_PATH ${FREERTOS_KERNEL_PATH}/${FREERTOS_KERNEL_PORT_RELATIVE_PATH})
message("USING FreeRTOS PATH=${FREERTOS_KERNEL_PATH}")
message("USING FreeRTOS PORT PATH=${FREERTOS_PORT_PATH}")

# Import Pico SDK and FreeRTOS CMake configs (this is a nameless evil, do not look upon it)
include(pico_sdk_import.cmake)
include(${FREERTOS_PORT_PATH}/library.cmake)

project(${PROJECT_NAME}
        LANGUAGES C CXX ASM)

pico_sdk_init()

add_executable(${OUTPUT_NAME}
    src/main.cpp
    src/pico_led.c
    src/wifi.cpp
    src/network_time.cpp
    ${PICO_SDK_PATH}/lib/lwip/src/apps/sntp/sntp.c
)

target_include_directories(${OUTPUT_NAME} PUBLIC 
    ${FREERTOS_KERNEL_PATH}/include 
    include/ 
)

# This makes printf() work over the USB serial port
pico_enable_stdio_usb(${OUTPUT_NAME} 1)

if(${PICO_BOARD} STREQUAL "pico_w")
    target_link_libraries(${OUTPUT_NAME} 
        pico_cyw43_arch_lwip_sys_freertos
        FreeRTOS-Kernel-Heap4)
    target_compile_definitions(FreeRTOS-Kernel INTERFACE PICO_RP2040=1)
else()
    target_link_libraries(${OUTPUT_NAME} 
        FreeRTOS-Kernel-Heap4)
endif()

target_link_libraries(${OUTPUT_NAME}
    pico_stdlib
    pico_runtime
    hardware_rtc
    pico_stdio_usb)

add_compile_options(-save-temps=obj -fverbose-asm)
target_compile_options(FreeRTOS-Kernel-Heap4 INTERFACE -save-temps=obj -fverbose-asm)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${OUTPUT_NAME})
