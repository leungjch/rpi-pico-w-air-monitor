cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
project(my_project C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

if (TARGET tinyusb_device)
# rest of your project
add_executable(pico_air_monitor
    main.c
    bme280.c
)

target_include_directories(pico_air_monitor PRIVATE ${CMAKE_CURRENT_LIST_DIR} )

target_compile_definitions(pico_air_monitor PRIVATE
WIFI_SSID=\"${WIFI_SSID}\"
WIFI_PASSWORD=\"${WIFI_PASSWORD}\")

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(pico_air_monitor 
pico_stdlib
pico_cyw43_arch_lwip_threadsafe_background
hardware_i2c
pico_lwip_mqtt
)

pico_enable_stdio_usb(pico_air_monitor 1)
pico_enable_stdio_uart(pico_air_monitor 0)


# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(pico_air_monitor)
elseif(PICO_ON_DEVICE)
message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in SDK")
endif()
