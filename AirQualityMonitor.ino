/***************************************************
 * Air Quality Monitoring System using ESP8266, Blynk, DHT11, and MQ-135
 * 
 * - Reads temperature and humidity from DHT11
 * - Reads air quality data from MQ-135 sensor
 * - Sends data to Blynk IoT dashboard (Virtual Pins V0, V1, V2)
 * - Triggers Blynk events for alerts
 ***************************************************/

// ==== BLYNK CONFIGURATION ====
#define BLYNK_TEMPLATE_ID "Your_Blynk_Template_ID"
#define BLYNK_TEMPLATE_NAME "Air Quality Monitor"
#define BLYNK_AUTH_TOKEN "Your_Blynk_Auth_Token" // Replace with your actual Blynk Auth Token

// ==== LIBRARIES ====
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// ==== WIFI CONFIG ====
char ssid[] = "Your_WiFi_SSID";         // Replace with your WiFi SSID
char pass[] = "Your_WiFi_Password";     // Replace with your WiFi Password

// ==== SENSOR CONFIG ====
#define DHTPIN D4        // GPIO2 (D4 on NodeMCU)
#define DHTTYPE DHT11
#define MQ135_PIN A0     // Analog pin A0

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// ==== Function to Read Air Quality ====
int readAirQuality() {
  int sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(MQ135_PIN);
    delay(100);
  }
  return sum / 5;
}

// ==== Send Sensor Data to Blynk ====
void sendSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Retry in case of read failure
  int retries = 3;
  while ((isnan(temperature) || isnan(humidity)) && retries--) {
    delay(1000);
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
  }

  int air_quality = readAirQuality();
  float air_ppm = map(air_quality, 0, 1023, 0, 1000); // Simulated PPM mapping

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error: DHT sensor not responding!");
    return;
  }

  // Send to Blynk
  Blynk.virtualWrite(V0, air_ppm);
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);

  // Serial Monitor Debugging
  Serial.print("Temp: "); Serial.print(temperature);
  Serial.print("°C, Humidity: "); Serial.print(humidity);
  Serial.print("%, Air Quality: "); Serial.println(air_ppm);

  // Alerts
  if (air_ppm > 500) Blynk.logEvent("air_quality", "⚠️ High Pollution! Please take precautions.");
  if (temperature > 35) Blynk.logEvent("temperature_", "🔥 Room is too hot! Open windows or turn on a fan or AC.");
  if (humidity > 70) Blynk.logEvent("humidity_", "💧 High Humidity! Risk of mold growth.");
  if (humidity < 30) Blynk.logEvent("low_humidity_", "⚠️ Air is too dry! Consider using a humidifier.");
  if (air_ppm > 1000) Blynk.logEvent("toxic_gas", "☠️ Toxic gas detected! Evacuate immediately.");
}

// ==== Setup ====
void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" Connected to WiFi!");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  dht.begin();
  timer.setInterval(2000L, sendSensorData); // 2-second interval
}

// ==== Loop ====
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected! Reconnecting...");
    WiFi.begin(ssid, pass);
  }

  Blynk.run();
  timer.run();
}

