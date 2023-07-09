use rumqttc::{MqttOptions, AsyncClient, QoS, EventLoop};
use redis::{Commands, cmd};
use std::error::Error;
use serde::{Serialize, Deserialize};
use serde_json::from_str;
use std::time::Duration;

#[derive(Serialize, Deserialize, Debug)]
struct SensorData {
    temperature: f64,
    pressure: f64,
    humidity: f64,
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    let mqtt_broker = "broker.hivemq.com";
    let mqtt_port = 1883;
    let mqtt_client_name = "rust_client";

    let mut mqttoptions = MqttOptions::new(mqtt_client_name, mqtt_broker, mqtt_port);
    mqttoptions.set_keep_alive(Duration::from_millis(15000));

    let (mut client, mut eventloop) = AsyncClient::new(mqttoptions, 10);

    // Subscribing to topic
    let topic = "pico_bme280";
    client.subscribe(topic, QoS::AtLeastOnce).await.unwrap();

    // Creating redis client
    let redis_client = redis::Client::open("redis://0.0.0.0")?;
    let mut redis_connection = redis_client.get_connection()?;

    loop {
        let data = eventloop.poll().await.unwrap();
        println!("Received {:?}", data);
         match eventloop.poll().await.unwrap() {
             rumqttc::Event::Incoming(rumqttc::Packet::Publish(publish)) => {
                 let message = publish.payload;
                 println!("Message is {:?}", message);
                 let message_str = std::str::from_utf8(&message).unwrap();
                 let data: SensorData = from_str(message_str)?;

                // Store the message into RedisTimeSeries
                 let _ : () = cmd("TS.ADD")
                 .arg("TS:TEMPERATURE")
                 .arg("*")
                 .arg(data.temperature)
                 .query(&mut redis_connection)?;

                 let _ : () = cmd("TS.ADD")
                 .arg("TS:PRESSURE")
                 .arg("*")
                 .arg(data.pressure)
                 .query(&mut redis_connection)?;

                let _ : () = cmd("TS.ADD")
                .arg("TS:HUMIDITY")
                .arg("*")
                .arg(data.humidity)
                .query(&mut redis_connection)?;

                 println!("Received: {:?}", data);
             }
             _ => {}
         }
    }
}