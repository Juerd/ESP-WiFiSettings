/*
    WifiSettings basic example

    Source and further documentation available at
    https://github.com/Juerd/esp32-WiFiSettings
*/

#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiSettings.h>

void setup() {
    Serial.begin(115200);
    SPIFFS.begin(true);  // Will format on the first run after failing to mount

    // Use stored credentials to connect to your WiFi access point.
    // If no credentials are stored or if the access point is out of reach,
    // an access point will be started with a captive portal to configure WiFi.
    WiFiSettings.connect();
}

void loop() {
    // Your loop code here
}
