cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(USERHOME $ENV{HOME})
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(PICO_SDK_PATH "../pico-sdk/")
set(PICO_TOOLCHAIN_PATH "${USERHOME}/.pico-sdk/toolchain/${toolchainVersion}")
set(FREERTOS_KERNEL_PATH "FreeRTOS-Kernel")
set(FREERTOS_KERNEL_PORT_RELATIVE_PATH "portable/GCC/ARM_CM0")
set(FREERTOS_PORT_PATH ${FREERTOS_KERNEL_PATH}/${FREERTOS_KERNEL_PORT_RELATIVE_PATH})
set(PICO_BOARD pico_w CACHE STRING "Board type")
set(PROJECT_NAME freeRTOS_hello_world)
set(OUTPUT_NAME freeRTOS_hello_world)

message("USING FreeRTOS PATH=${FREERTOS_KERNEL_PATH}")
message("USING FreeRTOS PORT PATH=${FREERTOS_PORT_PATH}")

include(pico_sdk_import.cmake)
#include(FreeRTOS_Kernel_import.cmake)

add_subdirectory(pico-cpp)

project(${PROJECT_NAME}
        LANGUAGES C CXX ASM)

pico_sdk_init()

add_executable(${OUTPUT_NAME}
    src/main.cpp 
)

pico_enable_stdio_usb(${OUTPUT_NAME} 1)


target_link_libraries(${OUTPUT_NAME} 
    pico_stdlib
    pico_stdio_usb
    pico_runtime
    pico_stdio_usb
    pico_cyw43_arch_none
    FreeRTOS)

    
add_library(FreeRTOS STATIC
    ${FREERTOS_KERNEL_PATH}/event_groups.c
    ${FREERTOS_KERNEL_PATH}/list.c
    ${FREERTOS_KERNEL_PATH}/queue.c
    ${FREERTOS_KERNEL_PATH}/stream_buffer.c
    ${FREERTOS_KERNEL_PATH}/tasks.c
    ${FREERTOS_KERNEL_PATH}/timers.c
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
    ${FREERTOS_PORT_PATH}/port.c
    ${FREERTOS_PORT_PATH}/portasm.c
)

# Build FreeRTOS
target_include_directories(FreeRTOS PUBLIC
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_PORT_PATH}
    include/
)


target_include_directories(${OUTPUT_NAME} PUBLIC 
    ${FREERTOS_KERNEL_PATH}/include 
    ${FREERTOS_PORT_PATH}
    include/ 
)

# target_include_directories(FreeRTOS PUBLIC 
#     FreeRTOS-Kernel/include 
#     include/ 
#     ${PICO_SDK_PATH}/src/common/pico_base_headers/include
#     ${PICO_SDK_PATH}/src/rp2040/pico_platform/include
#     ${PICO_SDK_PATH}/src/rp2_common/pico_platform_compiler/include
#     ${PICO_SDK_PATH}/src/rp2_common/pico_platform_sections/include
#     ${PICO_SDK_PATH}/src/rp2_common/pico_platform_panic/include
#     ${PICO_SDK_PATH}/src/rp2_common/hardware_sync/include
#     ${PICO_SDK_PATH}/src/rp2_common/hardware_sync_spin_lock/include
#     ${PICO_SDK_PATH}/src/rp2_common/hardware_base/include
#     ${PICO_SDK_PATH}/src/rp2_common/hardware_clocks/include
#     ${PICO_SDK_PATH}/src/rp2_common/hardware_exception/include
#     ${PICO_SDK_PATH}/src/rp2040/pico_platform/include/pico/platform
#     ${PICO_SDK_PATH}/src/rp2040/pico_platform/include
#     ${PICO_SDK_PATH}/src/rp2040/hardware_regs/include
#     ${PICO_SDK_PATH}/src/rp2040/hardware_structs/include
#     ${CMAKE_BINARY_DIR}/generated/pico_base
#     FreeRTOS-Kernel/portable/GCC/ARM_CM0
# )



# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${OUTPUT_NAME})
