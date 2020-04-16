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
        String string(String name, String init = "", String label = "");
        String string(String name, unsigned int max_length, String init = "", String label = "");
        long integer(String name, long init = 0, String label = "");
        long integer(String name, long min, long max, long init = 0, String label = "");

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

/*class WiFiConfigString: public String {
    public:
        WiFiConfigString(const char *p) : String(p) {}
        WiFiConfigString(const String &s) : String(s) {}
        String name;
    private:
};
*/

extern WiFiConfigClass WiFiConfig;

#endif
