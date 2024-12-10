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
set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

add_subdirectory(pico-cpp)

project(freeRTOS_hello_world C CXX ASM)

pico_sdk_init()

set(OUTPUT_NAME freeRTOS_hello_world)


add_executable(${OUTPUT_NAME}
    src/main.cpp 
)

pico_enable_stdio_usb(${OUTPUT_NAME} 1)


# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(${OUTPUT_NAME} 
    pico_stdlib
    pico_stdio_usb
    # pico_runtime
    FreeRTOS)
    # pico-cpp)
    
add_library(FreeRTOS STATIC FreeRTOS-Kernel/croutine.c
    FreeRTOS-Kernel/event_groups.c
    FreeRTOS-Kernel/list.c
    FreeRTOS-Kernel/queue.c
    FreeRTOS-Kernel/stream_buffer.c
    FreeRTOS-Kernel/tasks.c
    FreeRTOS-Kernel/timers.c 
    FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/port.c 
    FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/portASM.S 
    FreeRTOS-Kernel/portable/MemMang/heap_4.c 
    src/FreeRTOS_Support_Files/IdleMemory.c)

target_compile_definitions(FreeRTOS PUBLIC
    PICO_RP2040=1
    PICO_ON_DEVICE=1
)


target_include_directories(FreeRTOS PUBLIC 
    FreeRTOS-Kernel/include 
    include/ 
    ${PICO_SDK_PATH}/src/common/pico_base_headers/include
    ${PICO_SDK_PATH}/src/rp2040/pico_platform/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_compiler/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_sections/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_platform_panic/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_sync/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_sync_spin_lock/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_base/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_clocks/include
    ${PICO_SDK_PATH}/src/rp2_common/hardware_exception/include
    ${PICO_SDK_PATH}/src/rp2040/pico_platform/include/pico/platform
    ${PICO_SDK_PATH}/src/rp2040/pico_platform/include
    ${PICO_SDK_PATH}/src/rp2040/hardware_regs/include
    ${PICO_SDK_PATH}/src/rp2040/hardware_structs/include
    ${CMAKE_BINARY_DIR}/generated/pico_base
    FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/include
)



target_link_libraries(${OUTPUT_NAME} 
    FreeRTOS 
    # pico-cpp 
    pico_stdio_usb
    pico_cyw43_arch_none
    hardware_exception)


# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${OUTPUT_NAME})

# add_custom_command(TARGET hello_world
#         POST_BUILD
#         COMMAND arm-none-eabi-size hello_world.elf
#         )

        