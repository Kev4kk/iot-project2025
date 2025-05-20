#include <Arduino.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define I2S_WS      25  
#define I2S_SD      32 
#define I2S_SCK     26  


const char* ssid = "firstAID25";
const char* password = "iotempire";


const char* mqtt_server = "192.168.14.1";
const char* mqtt_topic = "mits/heli";

WiFiClient espClient;
PubSubClient client(espClient);

void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ALL_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}


void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to wifi:");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wifi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}


void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection");
    if (client.connect("ESP32MicClient")) {
      Serial.println("Connected");
    } else {
      Serial.print("Error: ");
      Serial.print(client.state());
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  setupWiFi();
  setupI2S();
  client.setServer(mqtt_server, 1883);
}


void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  int32_t sample = 0;
  size_t bytes_read;
  i2s_read(I2S_NUM_0, &sample, sizeof(sample), &bytes_read, portMAX_DELAY);

  if (bytes_read > 0) {
    int level = abs(sample) >> 16;  
    Serial.print("Sound level: ");
    Serial.println(level);

    char msg[16];
    itoa(level, msg, 10);
    client.publish(mqtt_topic, msg);
  }

  delay(200);
}