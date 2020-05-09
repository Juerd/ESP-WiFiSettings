/*
    WifiSettings advanced example

    Demonstrates callback functions and custom variables
    to be saved through WifiSettings.

    Source and further documentation available at
    https://github.com/Juerd/ESP-WiFiSettings

    Note: this example is written for ESP32.
    For ESP8266, use LittleFS.begin() instead of SPIFFS.begin(true).
*/

#include <SPIFFS.h>
#include <WiFiSettings.h>

// Status LED
const uint32_t LED_PIN = 2;
#define LED_ON  LOW
#define LED_OFF HIGH

void setup() {
    Serial.begin(115200);
    SPIFFS.begin(true);  // Will format on the first run after failing to mount

    pinMode(LED_PIN, OUTPUT);

    // Set custom callback functions
    WiFiSettings.onSuccess  = []() {
        digitalWrite(LED_PIN, LED_ON); // Turn LED on
    };
    WiFiSettings.onFailure  = []() {
        digitalWrite(LED_PIN, LED_OFF); // Turn LED off
    };
    WiFiSettings.onWaitLoop = []() {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle LED
        return 500; // Delay next function call by 500ms
    };

    // Callback functions do not have to be lambda's, e.g.
    // WiFiSettings.onPortalWaitLoop = blink;

    // Define custom settings saved by WifiSettings
    // These will return the default if nothing was set before
    String host = WiFiSettings.string( "server_host", "default.example.org");
    int    port = WiFiSettings.integer("server_port", 443);

    // Connect to WiFi with a timeout of 30 seconds
    // Launches the portal if the connection failed
    WiFiSettings.connect(true, 30);
}

void loop() {
    // Your loop code here
}
