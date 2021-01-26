#ifndef WiFiSettings_h
#define WiFiSettings_h

#include <Arduino.h>
#include <functional>

class WiFiSettingsClass {
    public:
        typedef std::function<void(void)> TCallback;
        typedef std::function<int(void)> TCallbackReturnsInt;
        typedef std::function<void(String&)> TCallbackString;

        WiFiSettingsClass();
        void begin();
        bool connect(bool portal = true, int wait_seconds = 30);
        void portal();
        String string(const String& name, const String& init = "", const String& label = "");
        String string(const String& name, unsigned int max_length, const String& init = "", const String& label = "");
        String string(const String& name, unsigned int min_length, unsigned int max_length, const String& init = "", const String& label = "");
        long integer(const String& name, long init = 0, const String& label = "");
        long integer(const String& name, long min, long max, long init = 0, const String& label = "");
        bool checkbox(const String& name, bool init = false, const String& label = "");
        void html(const String& tag, const String& contents, bool escape = true);
        void heading(const String& contents, bool escape = true);
        void warning(const String& contents, bool escape = true);
        void info(const String& contents, bool escape = true);

        String hostname;
        String password;
        bool secure;
        String language;

        TCallback onConnect;
        TCallbackReturnsInt onWaitLoop;
        TCallback onSuccess;
        TCallback onFailure;
        TCallback onPortal;
        TCallback onPortalView;
        TCallbackString onUserAgent;
        TCallback onConfigSaved;
        TCallback onRestart;
        TCallback onPortalWaitLoop;
    private:
        bool begun;
};

extern WiFiSettingsClass WiFiSettings;

#endif
