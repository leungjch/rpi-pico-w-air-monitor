# Indoor Air Monitoring with Raspberry Pi Pico W, BME280 and MQTT

This project is a simple indoor air monitoring system based on the Raspberry Pi Pico W, the BME280 sensor and MQTT. The Pico W is a microcontroller board with WiFi and the BME280 is a sensor for temperature, humidity and air pressure. The data is sent to a MQTT broker and a Redis instance. The data can be visualized with Grafana.

# Setup

## Raspberry Pi Pico W

Go to `pico`:
```
cd pico
```

Make build directory and configure cmake:
```
mkdir build
cd build
cmake -DPICO_BOARD=pico_w -DWIFI_SSID={YOUR_WIFI_SSD} -DWIFI_PASSW
ORD={YOUR_WIFI_PASSWORD} -DPICO_SDK_PATH={PATH_TO_PICO_SDK} ..
```

Build:
```
make pico_air_monitor
```

Follow steps in [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) to flash the build files to the Pico W.

## Server
The backend is a MQTT broker and a Rust server that subscribes to this MQTT broker and stores the data in a Redis instance. A Grafana instance is setup with the Redis data source to visualize the data.

