/*
    WifiSettings Arduino OTA example

    Demonstrates how to run Arduino OTA in tandem with WifiSettings,
    using the WifiSettings credentials.

    Source and further documentation available at
    https://github.com/Juerd/ESP-WiFiSettings

    Note: this example is written for ESP32.
    For ESP8266, use LittleFS.begin() instead of SPIFFS.begin(true).
*/

#include <SPIFFS.h>
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

    // Force WPA secured WiFi for the software access point.
    // Because OTA is remote code execution (RCE) by definition, the password
    // should be kept secret. By default, WiFiSettings will become an insecure
    // WiFi access point and happily tell anyone the password. The password
    // will instead be provided on the Serial connection, which is a bit safer.
    WiFiSettings.secure = true;

    // Set callbacks to start OTA when the portal is active
    WiFiSettings.onPortal = []() {
        setup_ota();
    };
    WiFiSettings.onPortalWaitLoop = []() {
        ArduinoOTA.handle();
    };

    // Use stored credentials to connect to your WiFi access point.
    // If no credentials are stored or if the access point is out of reach,
    // an access point will be started with a captive portal to configure WiFi.
    WiFiSettings.connect();

    Serial.print("Password: ");
    Serial.println(WiFiSettings.password);

    setup_ota();  // If you also want the OTA during regular execution
}

void loop() {
    ArduinoOTA.handle();  // If you also want the OTA during regular execution

    // Your loop code here
}
