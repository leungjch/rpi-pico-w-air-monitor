# Indoor Air Monitoring with Raspberry Pi Pico W, BME280 and MQTT

This project is a simple indoor air monitoring system based on the Raspberry Pi Pico W, the BME280 sensor and MQTT. The Pico W is a microcontroller board with WiFi and the BME280 is a sensor for temperature, humidity and air pressure. The data is sent to a MQTT broker and a Redis instance. The data can be visualized with Grafana.

# Setup

## Raspberry Pi Pico W

Go to `pico`:
```
cd pico
```

Configure cmake with the following command:

```
cmake -DPICO_BOARD=pico_w -DWIFI_SSID={YOUR_WIFI_SSD} -DWIFI_PASSW
ORD={YOUR_WIFI_PASSWORD}
```

Build:
```
mkdir build
cd build
make
```

Follow steps in [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) to flash the Pico W.