#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

// Pin Configuration
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi Configuration
const char *ssid = "BONVOUYAGE"; // Replace with your Wi-Fi network name
const char *password = "Qwerty123456"; // Replace with your Wi-Fi password

// API Configuration
const char *serverUrl = "https://climate-api-bsu.vercel.app/climate";
const char *fingerprint = "BB 8B 2C D9 45 95 D5 D8 FC 8A 8D 67 56 71 D5 99 AD F6 D5 FE";


// Variables for storing humidity and temperature values
float h; // Humidity
float t; // Temperature

// Function to log GET request
void logGETRequest(String url) {
    Serial.println("Logging GET Request...");
    Serial.print("URL: ");
    Serial.println(url);
}

// Setup function
void setup() {
    pinMode(D8, OUTPUT); // Blue LED
    pinMode(D0, OUTPUT); // Green LED
    pinMode(D1, OUTPUT); // Red LED

    Serial.begin(9600);

    dht.begin();

    connectToWiFi();
}

// Loop function
void loop() {
    h = dht.readHumidity();
    t = dht.readTemperature();

    // Control LEDs based on temperature
    if (t <= 15) {
        digitalWrite(D8, HIGH); // Blue LED behavior
    } else if (t > 15 && t <= 25) {
        digitalWrite(D0, HIGH); // Green LED behavior
    } else {
        digitalWrite(D1, HIGH); // Red LED behavior
    }

    if (WiFi.status() == WL_CONNECTED) {
        sendSensorData();
        delay(10000); // Delay of 10 seconds before sending data again
    }

    // Turn off LEDs after 5 seconds
    delay(5000);
    digitalWrite(D1, LOW);
    digitalWrite(D8, LOW);
    digitalWrite(D0, LOW);

    // Delay for 5 seconds before starting the loop again
    delay(5000);
}

void sendSensorData() {
    String url = "https://climate-api-bsu.vercel.app";
    logGETRequest(url);

    String clientUrl = serverUrl;

    clientUrl += "?temp=";
    clientUrl += String(t);
    clientUrl += "&hum=";
    clientUrl += String(h);

    WiFiClientSecure client;
    HTTPClient http;

    client.setInsecure();
    client.setFingerprint(fingerprint);

    if (!client.connect(url, 443)) { //works!
      Serial.println("HTTPS connection failed");
      return;
    }

    http.begin(client, clientUrl);

    int httpCode = http.POST("");

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.print("Response: ");
            Serial.println(payload);
        } else {
            String errorDetails = http.errorToString(httpCode).c_str();
            Serial.print("[HTTP] POST... failed, error code: ");
            Serial.print(httpCode);
            Serial.print(", error: ");
            Serial.println(errorDetails);
            String payload = http.getString();
            Serial.print("Payload: ");
            Serial.println(payload);
        }
    } else {
        String errorDetails = http.errorToString(httpCode).c_str();
        Serial.printf("[HTTP] POST... failed, error code: %d, error: %s\n", httpCode, errorDetails);
        String payload = http.getString();
        Serial.print("Payload: ");
        Serial.println(payload);
    }
    http.end();
}

// Function to connect to Wi-Fi
void connectToWiFi() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print("connecting... ");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
