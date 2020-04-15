#ifndef WiFiConfig_h
#define WiFiConfig_h

#include <Arduino.h>
#include <functional>

class WiFiConfigClass {
    public:
        typedef std::function<void(void)> TCallback;
        typedef std::function<int(void)> TCallbackInt;

        WiFiConfigClass();
        bool connect(bool portal = true, int wait_seconds = 30);
        void portal();
        String hostname;

        TCallback onConnect;
        TCallbackInt onWaitLoop;
        TCallback onSuccess;
        TCallback onFailure;
        TCallback onPortal;
        TCallback onConfigSaved;
        TCallback onRestart;
        TCallback onPortalWaitLoop;
    private:
};

extern WiFiConfigClass WiFiConfig;

#endif
