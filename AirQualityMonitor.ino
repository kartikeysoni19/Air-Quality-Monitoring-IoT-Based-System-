//#define BLYNK_TEMPLATE_ID "TMPL3SOmkO2wF"
//#define BLYNK_TEMPLATE_NAME "Air Quality Monitor"
//#define BLYNK_AUTH_TOKEN "uVRSgEB-e91lbTn064yzU5bDxr54pgvZ"

#define BLYNK_TEMPLATE_ID "TMPL3RaBznYaj"
#define BLYNK_TEMPLATE_NAME "Air Quality Monito"
#define BLYNK_AUTH_TOKEN "8i0uEbKQHJ8mT5KPNMxcyIjWSycdgOsl"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

char ssid[] = "redmi";  // Replace with your WiFi SSID
char pass[] = "1234567890"; // Replace with your WiFi Password

#define DHTPIN D4   // GPIO2 (D4) for DHT11
#define DHTTYPE DHT11
#define MQ135_PIN A0 // Analog pin A0 for MQ-135

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

int readAirQuality() {
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += analogRead(MQ135_PIN);
        delay(100);
    }
    return sum / 5;
}

void sendSensorData() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    // Retry in case of sensor read failure
    int retries = 3;
    while ((isnan(temperature) || isnan(humidity)) && retries--) {
        delay(1000);
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();
    }

    int air_quality = readAirQuality(); // Read MQ-135 sensor value
    float air_ppm = map(air_quality, 0, 1023, 0, 1000); // Convert to PPM

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Error: DHT sensor not responding!");
        return;
    }

    // Send values to Blynk Virtual Pins
    Blynk.virtualWrite(V0, air_ppm);
    Blynk.virtualWrite(V1, temperature);
    Blynk.virtualWrite(V2, humidity);

    Serial.print("Temp: "); Serial.print(temperature);
    Serial.print("°C, Humidity: "); Serial.print(humidity);
    Serial.print("%, Air Quality: "); Serial.println(air_ppm);

    // 🔴 ALERT NOTIFICATIONS 🔴
    if (air_ppm > 500) Blynk.logEvent("air_quality", "⚠️ High Pollution! Please take precautions.");
    if (temperature > 35) Blynk.logEvent("temperature_", "🔥 Room is too hot! Open windows or turn on a fan or AC.");
    if (humidity > 70) Blynk.logEvent("humidity_", "💧 High Humidity! Risk of mold growth.");
    if (humidity < 30) Blynk.logEvent("low_humidity_", "⚠️ Air is too dry! Consider using a humidifier.");
    if (air_ppm > 1000) Blynk.logEvent("toxic_gas", "☠️ Toxic gas detected! Evacuate immediately.");
}

void setup() {
    Serial.begin(115200);
    
    WiFi.begin(ssid, pass);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("Connected to WiFi!");

    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    
    dht.begin();
    timer.setInterval(2000L, sendSensorData);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi Disconnected! Reconnecting...");
        WiFi.begin(ssid, pass);
    }
    Blynk.run();
    timer.run();
}
