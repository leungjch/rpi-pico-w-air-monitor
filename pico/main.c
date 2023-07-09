#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "lwip/apps/mqtt_priv.h"
#include "bme280.h"

// set mqtt broker url to broker.hivemq.com
const char *MQTT_BROKER_URL = "3.122.167.216";

#ifndef LWIP_MQTT_EXAMPLE_IPADDR_INIT
#if LWIP_IPV4
#define LWIP_MQTT_EXAMPLE_IPADDR_INIT = IPADDR4_INIT(PP_HTONL(IPADDR_LOOPBACK))
#else
#define LWIP_MQTT_EXAMPLE_IPADDR_INIT
#endif
#endif

static ip_addr_t mqtt_ip LWIP_MQTT_EXAMPLE_IPADDR_INIT;

typedef struct MQTT_CLIENT_DATA_T
{
  mqtt_client_t *mqtt_client_inst;
  struct mqtt_connect_client_info_t mqtt_client_info;
  uint8_t data[MQTT_OUTPUT_RINGBUF_SIZE];
  uint8_t topic[100];
  uint32_t len;
  bool playing;
  bool newTopic;
} MQTT_CLIENT_DATA_T;

MQTT_CLIENT_DATA_T *mqtt;

struct mqtt_connect_client_info_t mqtt_client_info =
    {
        "test",
        "juleung", /* user */
        NULL,      /* pass */
        15,        /* keep alive */
        NULL,      /* will_topic */
        NULL,      /* will_msg */
        0,         /* will_qos */
        0          /* will_retain */
#if LWIP_ALTCP && LWIP_ALTCP_TLS
        ,
        NULL
#endif
};

/* Called when publish is complete either with sucess or failure */
void mqtt_pub_request_cb(void *arg, err_t result)
{
  if (result != ERR_OK)
  {
    printf("Publish result not OK: %d\n", result);
  }
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
  printf("mqtt_incoming_publish_cb\n");
  MQTT_CLIENT_DATA_T *mqtt_client = (MQTT_CLIENT_DATA_T *)arg;
  // strcpy(mqtt_client->topic, topic);
  strcpy((char *)mqtt_client->topic, (char *)topic);
}

static void mqtt_request_cb(void *arg, err_t err)
{
  MQTT_CLIENT_DATA_T *mqtt_client = (MQTT_CLIENT_DATA_T *)arg;

  LWIP_PLATFORM_DIAG(("MQTT client \"%s\" request cb: err %d\n", mqtt_client->mqtt_client_info.client_id, (int)err));
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
  MQTT_CLIENT_DATA_T *mqtt_client = (MQTT_CLIENT_DATA_T *)arg;
  LWIP_UNUSED_ARG(client);

  LWIP_PLATFORM_DIAG(("MQTT client \"%s\" connection cb: status %d\n", mqtt_client->mqtt_client_info.client_id, (int)status));

  if (status == MQTT_CONNECT_ACCEPTED)
  {
    printf("MQTT_CONNECT_ACCEPTED\n");
  }
}

void do_connect(MQTT_CLIENT_DATA_T *mqtt_client, void *arg)
{
  err_t err;
  cyw43_arch_lwip_begin();
  err = mqtt_client_connect(mqtt_client->mqtt_client_inst, &mqtt_ip, MQTT_PORT, mqtt_connection_cb, arg, &mqtt_client->mqtt_client_info);
  cyw43_arch_lwip_end();
  if (err != ERR_OK)
  {
    printf("mqtt_connect return %d\n", err);
  }
}

err_t example_publish(mqtt_client_t *client, void *arg)
{

  const char *pub_payload = "Picow MQTT";
  err_t err;
  u8_t qos = 2;    /* 0 1 or 2, see MQTT specification */
  u8_t retain = 0; /* No don't retain such crappy payload... */
  cyw43_arch_lwip_begin();
  err = mqtt_publish(client, "pico", pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, arg);
  cyw43_arch_lwip_end();
  if (err != ERR_OK)
  {
    printf("Publish err: %d\n", err);
  }

  return err;
}

int main()
{
  stdio_init_all();

  if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA))
  {
    printf("Wi-Fi init failed");
    return -1;
  }

  cyw43_arch_enable_sta_mode();

  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
  {
    printf("failed to connect.\n");
    return 1;
  }
  else
  {
    printf("Connected.\n");
  }

  // Initialize mqtt client
  mqtt = (MQTT_CLIENT_DATA_T *)calloc(1, sizeof(MQTT_CLIENT_DATA_T));
  if (!mqtt)
  {
    printf("Failed to allocate memory for mqtt client\n");
    return -1;
  }

  mqtt->playing = false;
  mqtt->newTopic = false;
  mqtt->mqtt_client_info = mqtt_client_info;

  mqtt->mqtt_client_inst = mqtt_client_new();

  if (!ip4addr_aton(MQTT_BROKER_URL, &mqtt_ip))
  {
    printf("ip error\n");
    return 0;
  }

  // Connect to mqtt broker
  err_t err = mqtt_client_connect(mqtt->mqtt_client_inst,
                                  &mqtt_ip, MQTT_PORT,
                                  mqtt_connection_cb, LWIP_CONST_CAST(void *, &mqtt->mqtt_client_info),
                                  &mqtt->mqtt_client_info);
  if (err != ERR_OK)
  {
    printf("mqtt_connect return %d\n", err);
    return -1;
  }

  // useful information for picotool
  bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
  bi_decl(bi_program_description("bme280 I2C example for the Raspberry Pi Pico"));

  printf("Hello, bme280! Reading temperaure and pressure values from sensor...\n");

  // I2C is "open drain", pull ups to keep signal high when no data is being sent
  i2c_init(i2c_default, 100 * 1000);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

  // configure bme280
  bme280_init();

  sleep_ms(500); // sleep so that data polling and register update don't collide

  while (true)
  {

    // retrieve fixed compensation params
    struct bme280_calib_param params;
    bme280_get_calib_params(&params);

    int32_t raw_temperature;
    int32_t raw_pressure;
    int32_t raw_humidity;

    // Turn on LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(250);


    bme280_read_raw(&raw_temperature, &raw_pressure, &raw_humidity);
    int32_t temperature = bme280_convert_temp(raw_temperature, &params);
    int32_t pressure = bme280_convert_pressure(raw_pressure, raw_temperature, &params);
    int32_t humidity = bme280_convert_humidity(raw_humidity, raw_temperature, &params);
    printf("Pressure = %.3f kPa\n", pressure / 1000.f);
    printf("Temp. = %.2f C\n", temperature / 100.f);
    printf("Humidity = %.2f %%\n", (double)humidity / (double)1024.0);

    // publish temperature and pressure values and humdidity to MQTT broker
    char buf[100];
    sprintf(buf, "{\"temperature\": %.2f, \"pressure\": %.3f, \"humidity\": %.2f}", temperature / 100.f, pressure / 1000.f, (double)humidity / (double)1024.0);

    do_connect(mqtt, mqtt);
    // example_publish(mqtt->mqtt_client_inst, mqtt);
    mqtt_publish(mqtt->mqtt_client_inst, "pico_bme280", buf, strlen(buf), 0, 0, mqtt_pub_request_cb, mqtt);

    // Turn off LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(250);
  }
}