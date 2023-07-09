# FREERTOS_KERNEL_PATH=/opt/freertos-kernel-10.5.1

cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
include(FreeRTOS_Kernel_import.cmake)
project(my_project C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

if (TARGET tinyusb_device)
# rest of your project
add_executable(hello_world
    main.c
    bme280.c
)

target_include_directories(hello_world PRIVATE ${CMAKE_CURRENT_LIST_DIR} )

target_compile_definitions(hello_world PRIVATE
WIFI_SSID=\"${WIFI_SSID}\"
WIFI_PASSWORD=\"${WIFI_PASSWORD}\")

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(hello_world 
pico_stdlib
pico_cyw43_arch_lwip_threadsafe_background
FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
hardware_i2c
pico_lwip_mqtt
)

pico_enable_stdio_usb(hello_world 1)
pico_enable_stdio_uart(hello_world 0)


# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hello_world)
elseif(PICO_ON_DEVICE)
message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in SDK")
endif()
