#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
#include <WiFiS3.h>
#include <PubSubClient.h>

// Sensor & display
Adafruit_BME280 sensor;
ArduinoLEDMatrix matrix;

// Wi-Fi-oplysninger
const char* ssid     = "h4prog";         // Dit SSID
const char* password = "1234567890";     // Dit Wi-Fi-kodeord

// MQTT-opsætning
const char* mqtt_server = "192.168.110.11";
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "weather/gruppe10";

// Netværks-klienter
WiFiClient   wifiClient;              
PubSubClient mqttClient(wifiClient);

// publish interval
const unsigned long publishInterval = 30UL * 1000UL;  // 30 000 ms
unsigned long lastPublish = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  //initiere BME280 på 0x76
  if (!sensor.begin(0x76)) {
    Serial.println("Cant find sensor, check wiring!");
    while (1);
  }
  
  // initiere LED matrix
  matrix.begin();
  Serial.println("Sensor and LED matrix ready!");

  // connect til wifi
  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  delay(2000);
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(mqtt_server, mqtt_port);
}

// sikre connection og reconnecter
void ensureMqttConnected() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT at ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.println(mqtt_port);
    if (mqttClient.connect("arduinoR4Client")) {
      Serial.println("MQTT connected");
    } else {
      Serial.print("MQTT connect failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" – retry in 5s");
      delay(5000);
    }
  }
}

void loop() {
  // Keep MQTT alive
  if (!mqttClient.connected()) {
    ensureMqttConnected();
  }
  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastPublish >= publishInterval) {
    lastPublish = now;

    // læs sensors
    float temperature = sensor.readTemperature();
    float humidity    = sensor.readHumidity();
    float pressure    = sensor.readPressure() / 100.0F; // hPa

    // Display temp + humidity på LED matrix
    char tbuf[6], hbuf[6];
    dtostrf(temperature, 4, 1, tbuf);
    dtostrf(humidity,    4, 1, hbuf);
    char msg[12];
    snprintf(msg, sizeof(msg), "%sC %s%%", tbuf, hbuf);
    matrix.beginDraw();
      matrix.stroke(0xFFFFFFFF);
      matrix.textScrollSpeed(120);
      matrix.textFont(Font_5x7);
      matrix.beginText(0, 1, 0xFFFFFFFF);
      matrix.println(msg);
    matrix.endText(SCROLL_LEFT);
    matrix.endDraw();

    // log til serial monitor
    Serial.print("Temperature = "); Serial.print(temperature); Serial.println(" °C");
    Serial.print("Humidity    = "); Serial.print(humidity);    Serial.println(" %");
    Serial.print("Pressure    = "); Serial.print(pressure);    Serial.println(" hPa");
    Serial.println("--------------------------");

    // Publish JSON til MQTT 
    char payload[128];
    snprintf(payload, sizeof(payload),
             "{\"temp\":%.2f,\"hum\":%.2f,\"pres\":%.2f}",
             temperature, humidity, pressure);
    bool ok = mqttClient.publish(mqtt_topic, payload);
    Serial.print("Published to "); Serial.print(mqtt_topic);
    Serial.print(" → "); Serial.println(ok ? "OK" : "FAIL");
  }
}
