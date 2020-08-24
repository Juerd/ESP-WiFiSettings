#ifndef WiFiSettings_h
#define WiFiSettings_h

#include <Arduino.h>
#include <functional>

class WiFiSettingsClass {
    public:
        typedef std::function<void(void)> TCallback;
        typedef std::function<int(void)> TCallbackInt;

        WiFiSettingsClass();
        void begin();
        bool connect(bool portal = true, int wait_seconds = 30, IPAddress static_ip = INADDR_NONE, IPAddress gateway = INADDR_NONE, IPAddress subnet = INADDR_NONE, IPAddress dns = INADDR_NONE);
        void portal();
        String string(const String& name, const String& init = "", const String& label = "");
        String string(const String& name, unsigned int max_length, const String& init = "", const String& label = "");
        String string(const String& name, unsigned int min_length, unsigned int max_length, const String& init = "", const String& label = "");
        long integer(const String& name, long init = 0, const String& label = "");
        long integer(const String& name, long min, long max, long init = 0, const String& label = "");
        bool checkbox(const String& name, bool init = false, const String& label = "");

        String hostname;
        String password;
        bool secure;

        TCallback onConnect;
        TCallbackInt onWaitLoop;
        TCallback onSuccess;
        TCallback onFailure;
        TCallback onPortal;
        TCallback onConfigSaved;
        TCallback onRestart;
        TCallback onPortalWaitLoop;
    private:
        bool begun;
};

extern WiFiSettingsClass WiFiSettings;

#endif
