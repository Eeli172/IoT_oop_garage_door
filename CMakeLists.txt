cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico_w)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(Garage_Door C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/gpios/gpios.cpp
        src/gpios/encoder/encoder.cpp
        src/gpios/leds/leds.cpp
        src/system/sys.cpp
        src/system/mqtt/mqtt.cpp
        src/system/wifi/wifi.cpp
        src/motor_run/motor_run.cpp
        src/interrupts/interrupts.cpp
)

pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
        src
        src/system/mqtt
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        pico_util
        hardware_pwm
        hardware_gpio
        hardware_uart
        hardware_i2c
        pico_cyw43_arch_lwip_threadsafe_background  # Wi-Fi + IPs
        pico_lwip_mqtt                             # lwIP MQTT library
)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)