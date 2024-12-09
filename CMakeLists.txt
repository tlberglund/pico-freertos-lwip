cmake_minimum_required(VERSION 3.12)


set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)

set(PICO_BOARD pico_w CACHE STRING "Board type")


# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)
add_subdirectory(pico-cpp)
project(hello_world)

# initialize the Pico SDK
pico_sdk_init()


add_executable(hello_world
    src/main.cpp 
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(hello_world 
    pico_stdlib
    pico_runtime
    FreeRTOS
    pico-cpp)
    
add_library(FreeRTOS STATIC FreeRTOS-Kernel/croutine.c
    FreeRTOS-Kernel/event_groups.c
    FreeRTOS-Kernel/list.c
    FreeRTOS-Kernel/queue.c
    FreeRTOS-Kernel/stream_buffer.c
    FreeRTOS-Kernel/tasks.c
    FreeRTOS-Kernel/timers.c 
    FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/port.c 
    FreeRTOS-Kernel/portable/MemMang/heap_4.c 
    src/FreeRTOS_Support_Files/IdleMemory.c)

target_compile_definitions(FreeRTOS PUBLIC
    PICO_RP2040=1
    PICO_ON_DEVICE=1
)
message("PICO_SDK_INCLUDE_DIRS=${PICO_SDK_INCLUDE_DIRS}")


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
    FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/include)

#    FreeRTOS-Kernel/portable/GCC/ARM_CM0/port.c 


# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hello_world)

target_link_libraries(hello_world 
    FreeRTOS 
    pico-cpp 
    pico_cyw43_arch_none
    hardware_exception)


add_custom_command(TARGET hello_world
        POST_BUILD
        COMMAND arm-none-eabi-size hello_world.elf
        )

        