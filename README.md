# Indoor Air Monitoring with Raspberry Pi Pico W, BME280 and MQTT

This project is a simple indoor air monitoring system based on the Raspberry Pi Pico W, the BME280 sensor and MQTT. The Pi Pico W reads from the BME280 sensor via I2C to obtain temperature, humidity and air pressure data. The data is sent to a MQTT broker which is forwarded to a Redis instance by a Rust backend. The data and Redis server health are visualized as a Grafana dashboard.

The Redis/Grafana/Rust backend are all hosted on an AWS t2.micro.

<img width="1440" alt="image" src="https://github.com/leungjch/pico-air-monitor/assets/28817028/e386def0-a351-4bdc-bc59-d87548fe8bbf">

![pico-air-diagram drawio (2)](https://github.com/leungjch/pico-air-monitor/assets/28817028/2cb7f79d-8172-460c-b546-ef4e6270e91c)


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

The Pico SDK can be cloned here: https://github.com/raspberrypi/pico-sdk

Build:
```
make pico_air_monitor
```

Follow steps in [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) to flash the build files to the Pico W.

## Server
The backend is a MQTT broker and a Rust server that subscribes to this MQTT broker and stores the data in a Redis instance. A Grafana instance is setup with the Redis data source to visualize the data.

docker-compose is used to setup the server. Go to `server`:
```
cd server
```
Start the server:

```
docker-compose up
```

The Grafana dashboard is available at `localhost:3000` and the Redis instance is at `localhost:6379`.
