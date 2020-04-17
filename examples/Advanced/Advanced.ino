/*
    WifiSettings advanced example

    Demonstrates callback functions and custom variables
    to be saved through WifiSettings.

    Source and further documentation available at
    https://github.com/Juerd/esp32-WiFiSettings
*/

#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiSettings.h>

void setup() {
    Serial.begin(115200);
    SPIFFS.begin(true);  // Will format on the first run after failing to mount

    // Set custom callback functions
    WiFiSettings.onSuccess  = []() {
        Serial.println("Successfully connected to WiFi");
    };
    WiFiSettings.onFailure  = []() {
        Serial.println("Failed to connect to WiFi!");
    };
    WiFiSettings.onWaitLoop = []() {
        Serial.print(".");
        return 500; // Delay next function call by 1000ms
    };

    // Callback functions do not have to be lambda's, e.g.
    // WiFiSettings.onPortalWaitLoop = []() { blink(); };

    // Define custom settings saved by WifiSettings
    // These will return the default if nothing was set before
    String host = WiFiSettings.string( "server_host", "default.example.org");
    // Integers have a min, max, and default
    int    port = WiFiSettings.integer("server_port", 0, 65535, 443);

    // Connect to WiFi with a timeout of 30 seconds
    // Launches the portal if the connection failed
    WiFiSettings.connect(true, 30);
}

void loop() {
    // Your loop code here
}
