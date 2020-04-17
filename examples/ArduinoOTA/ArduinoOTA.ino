/*
    WifiSettings Arduino OTA example

    Demonstrates how to run Arduino OTA in tandem with WifiSettings,
    using the WifiSettings credentials.

    Source and further documentation available at
    https://github.com/Juerd/esp32-WiFiSettings
*/

#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiSettings.h>
#include <ArduinoOTA.h>

// Start ArduinoOTA via WiFiSettings with the same hostname and password
void setup_ota() {
    ArduinoOTA.setHostname(WiFiSettings.hostname.c_str());
    ArduinoOTA.setPassword(WiFiSettings.password.c_str());
    ArduinoOTA.begin();
}

void setup() {
    Serial.begin(115200);
    SPIFFS.begin(true);  // Will format on the first run after failing to mount

    WiFiSettings.secure = true; // Force a secure portal

    // Set callbacks to start OTA when the portal is active
    WiFiSettings.onPortal = []() {
        setup_ota();
    };
    WiFiSettings.onPortalWaitLoop = []() {
        ArduinoOTA.handle();
    };

    Serial.print("Password: ");
    Serial.println(WiFiSettings.password);

    // Use stored credentials to connect to your WiFi access point.
    // If no credentials are stored or if the access point is out of reach,
    // an access point will be started with a captive portal to configure WiFi.
    WiFiSettings.connect();

    setup_ota();  // If you also want the OTA during regular execution
}

void loop() {
    ArduinoOTA.handle();  // If you also want the OTA during regular execution

    // Your loop code here
}
