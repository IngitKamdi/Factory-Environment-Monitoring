#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define WIFI_SSID "Freenet"            // Replace with your Wi-Fi SSID
#define WIFI_PASSWORD "69697070"    // Replace with your Wi-Fi Password
#define THINGSPEAK_API_KEY "OKD5PH3ECQ4JING3"     // Replace with your ThingSpeak API Key
#define THINGSPEAK_URL "http://api.thingspeak.com/update"

#define DHTPIN 4         // GPIO4 connected to DHT22
#define DHTTYPE DHT22    // DHT22 sensor type
#define MQ135_PIN 34     // GPIO34 for MQ135 Analog output (AOUT)
#define CZN15E_PIN 35    // GPIO35 for CZN-15E Analog output (AOUT)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Set ADC resolution and attenuation for ESP32
  analogReadResolution(12);       // 12-bit resolution (0-4095)
  analogSetAttenuation(ADC_11db); // Full-scale voltage range (0-3.3V)

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
}

void loop() {
  // Read Temperature and Humidity from DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Check if DHT22 readings failed and exit early
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT22 sensor!");
    return;
  }

  // Read Air Quality from MQ135
  int mq135Value = analogRead(MQ135_PIN);
  float airQualityVoltage = mq135Value * (3.3 / 4095.0);

  // Read Noise Level from CZN-15E
  int noiseValue = analogRead(CZN15E_PIN);
  float noiseVoltage = noiseValue * (3.3 / 4095.0);

  // Print readings to Serial Monitor
  Serial.print("Temperature: "); Serial.print(temperature); Serial.print(" °C, ");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.print(" %, ");
  Serial.print("Air Quality Voltage: "); Serial.print(airQualityVoltage); Serial.print(" V, ");
  Serial.print("Noise Voltage: "); Serial.print(noiseVoltage); Serial.println(" V");

  // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = THINGSPEAK_URL;
    url += "?api_key=" + String(THINGSPEAK_API_KEY);
    url += "&field1=" + String(temperature);
    url += "&field2=" + String(humidity);
    url += "&field3=" + String(airQualityVoltage);
    url += "&field4=" + String(noiseVoltage);

    http.begin(url); // Start connection to ThingSpeak
    int httpResponseCode = http.GET(); // Send GET request

    if (httpResponseCode > 0) {
      Serial.print("Data sent to ThingSpeak. Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error in sending data. Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end(); // End the HTTP connection
  } else {
    Serial.println("Wi-Fi not connected.");
  }

  delay(15000); // Send data every 15 seconds
}
