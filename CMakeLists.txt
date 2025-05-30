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
set(CMAKE_CXX_STANDAR 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

# REMOVE USO DE PROJECT_NAME
set(FREERTOS_KERNEL_PATH "C:/FreeRTOS-Kernel")

# Use nome literal do projeto
project(semaphore C CXX ASM)

include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

pico_sdk_init()

include_directories(${CMAKE_SOURCE_DIR}/libs)
file(GLOB LIBS "libs/*.c")

# Usa nome explícito do binário
add_executable(semaphore src/main.c ${LIBS})

target_compile_definitions(semaphore PRIVATE
    PICO_PRINTF_SUPPORT_FLOAT=1
    PICO_STDIO_ENABLE_PRINTF=1
)

target_link_libraries(semaphore
    pico_stdlib
    hardware_i2c
    hardware_adc
    hardware_pio
    hardware_clocks
    hardware_pwm
    FreeRTOS-Kernel 
    FreeRTOS-Kernel-Heap4
)

pico_generate_pio_header(semaphore ${CMAKE_CURRENT_LIST_DIR}/libs/ws2818b.pio)
pico_enable_stdio_uart(semaphore 1)
pico_enable_stdio_usb(semaphore 1)
pico_add_extra_outputs(semaphore)
