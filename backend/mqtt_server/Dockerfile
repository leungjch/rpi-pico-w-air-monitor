# Dockerfile
FROM rust:1.67

WORKDIR /usr/src/mqtt_server
COPY . .

RUN cargo install --path .

CMD ["mqtt_server"]
