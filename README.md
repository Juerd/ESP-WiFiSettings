# WiFi configuration manager for the ESP32 platform in the Arduino Framework

## Minimal usage

```
#include <SPIFFS.h>
#include <WiFiConfig.h>

void setup() {
    Serial.begin(115200);
    SPIFFS.begin(true);  // Will fail to mount and then format, on first run

    WiFiConfig.connect();
}
```

## Example with callbacks

```
void setup() {
    Serial.begin(115200);
    SPIFFS.begin(true);  // Will fail to mount and then format, on first run

    // Note that these examples call functions that you probably don't have.
    WiFiConfig.onSuccess  = []() { green(); }
    WiFiConfig.onFailure  = []() { red(); }
    WiFiConfig.onWaitLoop = []() { blue(); return 30; }  // delay 30 ms
    WiFiConfig.onPortalWaitLoop = []() { blink(); }
    WiFiConfig.connect(true, 30);
}
```

## Description

This is a very simple, and somewhat naive, WiFi configuration manager for
ESP32 programs written in the Arduino framework. It will allow you to
configure your WiFi network name (SSID) and password via a captive portal:
the ESP32 becomes an access point with a web based configuration page.

It was written for ease of use, not for extended functionality. For example,
restarting the microcontroller is the only way to leave the configuration
portal. A button to restart is provided in the web interface.

The library generates a random password to protect the portal with, but
it's only secured if you choose to do so by checking a checkbox. Of course,
the user can also pick their own password. "Hard coding" the password in
your program is a bad practice, and not supported.

The configuration is stored in files in the SPIFFS (SPI Flash FileSystem),
that are dumped in the root directory of the filesystem. Debug output
(including the password to the configuration portal) is written to `Serial`.

Only automatic IP address assignment (DHCP) is supported.

## Reference

### String WiFiConfig.hostname

Name to use as the hostname and SSID for the access point.

By default, this is set to "esp32-123456" where 123456 is the hexadecimal
representation of the device interface specific part of the ESP32's MAC
address, in reverse byte order.

The password for the access point can be set in the configuration portal.

### bool WiFiConfig.connect(bool portal = true, int wait_seconds = 30)

If no WiFi network is configured yet, starts the configuration portal.
In other cases, it will attempt to connect to the network, and wait until
either a connection is established, or `wait_seconds` has elapsed.
Returns `true` if connection succeeded.

By default, a failed connection (no connection established within the timeout)
will cause the configuration portal to be started. Given `portal = false`, it
will instead return `false`.

To wait forever until WiFi is connected, use `wait_seconds = -1`. In this case,
the value of `portal` is ignored.

Calls the following callbacks:

* WiFiConfig.onConnect
* WiFiConfig.onWaitLoop -> int (milliseconds to wait)
* WiFiConfig.onSuccess
* WiFiConfig.onFailure

### void WiFiConfig.portal()

Disconnects any active WiFi and turns the ESP32 into a captive portal with a
DNS server that works on every hostname.

Normally, this method is called by `.connect()`. To allow reconfiguration
after the initial configuration, you could call `.portal()` manually, for
example when a button is pressed during startup.

This method never ends. A restart is required to resume normal operation.

Calls the following callbacks:

* WiFiConfig.onPortal
* WiFiConfig.onPortalWaitLoop
* WiFiConfig.onConfigSaved
* WiFiConfig.onRestart
