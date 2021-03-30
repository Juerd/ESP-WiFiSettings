#include "WiFiSettings.h"
#ifdef ESP32
    #define ESPFS SPIFFS
    #define ESPMAC (Sprintf("%06" PRIx64, ESP.getEfuseMac() >> 24))
    #include <SPIFFS.h>
    #include <WiFi.h>
    #include <WebServer.h>
    #include <esp_task_wdt.h>
#elif ESP8266
    #define ESPFS LittleFS
    #define ESPMAC (Sprintf("%06" PRIx32, ESP.getChipId()))
    #include <LittleFS.h>
    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>
    #define WebServer ESP8266WebServer
    #define esp_task_wdt_reset wdt_reset
    #define wifi_auth_mode_t uint8_t    // wl_enc_type
    #define WIFI_AUTH_OPEN ENC_TYPE_NONE
    constexpr auto WIFI_AUTH_WPA2_ENTERPRISE = -1337; // not available on ESP8266
    #define setHostname hostname
    #define INADDR_NONE IPAddress(0,0,0,0)
#else
    #error "This library only supports ESP32 and ESP8266"
#endif
#include <DNSServer.h>
#include <limits.h>
#include <vector>
#include <WiFiSettings_strings.h>

WiFiSettingsLanguage::Texts _WSL_T;

#define Sprintf(f, ...) ({ char* s; asprintf(&s, f, __VA_ARGS__); String r = s; free(s); r; })

namespace {  // Helpers
    String slurp(const String& fn) {
        File f = ESPFS.open(fn, "r");
        String r = f.readString();
        f.close();
        return r;
    }

    bool spurt(const String& fn, const String& content) {
        File f = ESPFS.open(fn, "w");
        if (!f) return false;
        auto w = f.print(content);
        f.close();
        return w == content.length();
    }

    String pwgen() {
        const char* passchars = "ABCEFGHJKLMNPRSTUXYZabcdefhkmnorstvxz23456789-#@?!";
        String password = "";
        for (int i = 0; i < 16; i++) {
            // Note: no seed needed for ESP8266 and ESP32 hardware RNG
            password.concat( passchars[random(strlen(passchars))] );
        }
        return password;
    }

    String html_entities(const String& raw) {
        String r;
        for (unsigned int i = 0; i < raw.length(); i++) {
            char c = raw.charAt(i);
            if (c < '!' || c == '"' || c == '&' || c == '\'' || c == '<' || c == '>' || c == 0x7f) {
                // ascii control characters, html syntax characters, and space
                r += Sprintf("&#%d;", c);
            } else {
                r += c;
            }
        }
        return r;
    }

    struct WiFiSettingsParameter {
        String name;
        String label;
        String value;
        String init;
        long min = LONG_MIN;
        long max = LONG_MAX;

        String filename() { String fn = "/"; fn += name; return fn; }
        bool store() { return (name && name.length())? spurt(filename(), value): true; }
        void fill() { if (name && name.length()) value = slurp(filename()); }
        virtual void set(const String&) = 0;
        virtual String html() = 0;
    };

    struct WiFiSettingsString : WiFiSettingsParameter {
        virtual void set(const String& v) { value = v; }
        String html() {
            String h = F("<p><label>{label}:<br><input name='{name}' value='{value}' placeholder='{init}' minlength={min} maxlength={max}></label>");
            h.replace("{name}", html_entities(name));
            h.replace("{value}", html_entities(value));
            h.replace("{init}", html_entities(init));
            h.replace("{label}", html_entities(label));
            h.replace("{min}", String(min));
            h.replace("{max}", String(max));
            return h;
        }
    };

    struct WiFiSettingsInt : WiFiSettingsParameter {
        virtual void set(const String& v) { value = v; }
        String html() {
            String h = F("<p><label>{label}:<br><input type=number step=1 min={min} max={max} name='{name}' value='{value}' placeholder='{init}'></label>");
            h.replace("{name}", html_entities(name));
            h.replace("{value}", html_entities(value));
            h.replace("{init}", html_entities(init));
            h.replace("{label}", html_entities(label));
            h.replace("{min}", String(min));
            h.replace("{max}", String(max));
            return h;
        }
    };

    struct WiFiSettingsBool : WiFiSettingsParameter {
        virtual void set(const String& v) { value = v.length() ? "1" : "0"; }
        String html() {
            String h = F("<p><label class=c><input type=checkbox name='{name}' value=1{checked}> {label} ({default}: {init})</label>");
            h.replace("{name}", html_entities(name));
            h.replace("{default}", _WSL_T.init);
            h.replace("{checked}", value.toInt() ? " checked" : "");
            h.replace("{init}", init.toInt() ? "&#x2611;" : "&#x2610;");
            h.replace("{label}", html_entities(label));
            return h;
        }
    };

    struct WiFiSettingsHTML : WiFiSettingsParameter {
        // Raw HTML, not an actual parameter. The reason for the "if (name)"
        // in store and fill. Abuses several member variables for completely
        // different functionality.

        virtual void set(const String& v) { (void)v; }
        String html() {
            int space = value.indexOf(" ");

            String h = 
                (value ? "<" + value + ">" : "")
                +
                (min ? html_entities(label) : label)
                +
                (value ? "</" + (space >= 0 ? value.substring(0, space) : value) + ">" : "");
            return h;
        }
    };

    struct std::vector<WiFiSettingsParameter*> params;
}

String WiFiSettingsClass::string(const String& name, const String& init, const String& label) {
    begin();
    struct WiFiSettingsString* x = new WiFiSettingsString();
    x->name = name;
    x->label = label.length() ? label : name;
    x->init = init;
    x->fill();

    params.push_back(x);
    return x->value.length() ? x->value : x->init;
}

String WiFiSettingsClass::string(const String& name, unsigned int max_length, const String& init, const String& label) {
    String rv = string(name, init, label);
    params.back()->max = max_length;
    return rv;
}

String WiFiSettingsClass::string(const String& name, unsigned int min_length, unsigned int max_length, const String& init, const String& label) {
    String rv = string(name, init, label);
    params.back()->min = min_length;
    params.back()->max = max_length;
    return rv;
}

long WiFiSettingsClass::integer(const String& name, long init, const String& label) {
    begin();
    struct WiFiSettingsInt* x = new WiFiSettingsInt();
    x->name = name;
    x->label = label.length() ? label : name;
    x->init = init;
    x->fill();

    params.push_back(x);
    return (x->value.length() ? x->value : x->init).toInt();
}

long WiFiSettingsClass::integer(const String& name, long min, long max, long init, const String& label) {
    long rv = integer(name, init, label);
    params.back()->min = min;
    params.back()->max = max;
    return rv;
}

bool WiFiSettingsClass::checkbox(const String& name, bool init, const String& label) {
    begin();
    struct WiFiSettingsBool* x = new WiFiSettingsBool();
    x->name = name;
    x->label = label.length() ? label : name;
    x->init = String((int) init);
    x->fill();

    // Apply default immediately because a checkbox has no placeholder to
    // show the default, and other UI elements aren't sufficiently pretty.
    if (! x->value.length()) x->value = x->init;

    params.push_back(x);
    return x->value.toInt();
}

void WiFiSettingsClass::html(const String& tag, const String& contents, bool escape) {
    begin();
    struct WiFiSettingsHTML* x = new WiFiSettingsHTML();
    x->value = tag;
    x->label = contents;
    x->min = escape;

    params.push_back(x);
}

void WiFiSettingsClass::info(const String& contents, bool escape) {
    html(F("p class=i"), contents, escape);
}

void WiFiSettingsClass::warning(const String& contents, bool escape) {
    html(F("p class=w"), contents, escape);
}

void WiFiSettingsClass::heading(const String& contents, bool escape) {
    html("h2", contents, escape);
}


void WiFiSettingsClass::portal() {
    WebServer http(80);
    DNSServer dns;
    int num_networks = -1;
    begin();

    #ifdef ESP32
        WiFi.disconnect(true, true);    // reset state so .scanNetworks() works
    #else
        WiFi.disconnect(true);
    #endif

    Serial.println(F("Starting access point for configuration portal."));
    if (secure && password.length()) {
        Serial.printf("SSID: '%s', Password: '%s'\n", hostname.c_str(), password.c_str());
        WiFi.softAP(hostname.c_str(), password.c_str());
    } else {
        Serial.printf("SSID: '%s'\n", hostname.c_str());
        WiFi.softAP(hostname.c_str());
    }
    delay(500);
    dns.setTTL(0);
    dns.start(53, "*", WiFi.softAPIP());

    if (onPortal) onPortal();
    String ip = WiFi.softAPIP().toString();
    Serial.println(ip);

    auto redirect = [&http, &ip]() {
        // iPhone doesn't deal well with redirects to http://hostname/ and
        // will wait 40 to 60 seconds before succesful retry. Works flawlessly
        // with http://ip/ though.
        if (http.hostHeader() == ip) return false;

        http.sendHeader("Location", "http://" + ip + "/");
        // Anecdotally, some devices require a non-empty response body
        http.send(302, "text/plain", ip);
        return true;
    };

    const char* headers[] = {"User-Agent"};
    http.collectHeaders(headers, sizeof(headers) / sizeof(char*));

    http.on("/", HTTP_GET, [this, &http, &num_networks, &redirect]() {
        if (redirect()) return;

        String ua = http.header("User-Agent");
        bool interactive = !ua.startsWith(F("CaptiveNetworkSupport"));

        if (interactive && onPortalView) onPortalView();
        if (onUserAgent) onUserAgent(ua);

        http.setContentLength(CONTENT_LENGTH_UNKNOWN);
        http.send(200, "text/html");
        http.sendContent(F("<!DOCTYPE html>\n<meta charset=UTF-8><title>"));
        http.sendContent(html_entities(hostname));
        http.sendContent(F("</title>"
            "<meta name=viewport content='width=device-width,initial-scale=1'>"
            "<style>"
            "*{box-sizing:border-box} "
            "html{background:#444;font:10pt sans-serif}"
            "body{background:#ccc;color:black;max-width:30em;padding:1em;margin:1em auto}"
            "a:link{color:#000} "
            "label{clear:both}"
            "select,input:not([type^=c]){display:block;width:100%;border:1px solid #444;padding:.3ex}"
            "input[type^=s]{display:inline;width:auto;background:#de1;padding:1ex;border:1px solid #000;border-radius:1ex}"
            "[type^=c]{float:left;margin-left:-1.5em}"
            ":not([type^=s]):focus{outline:2px solid #d1ed1e}"
            ".w::before{content:'\\26a0\\fe0f'}"
            "p::before{margin-left:-2em;float:left;padding-top:1ex}"
            ".i::before{content:'\\2139\\fe0f'}"
            ".c{display:block;padding-left:2em}"
            ".w,.i{display:block;padding:.5ex .5ex .5ex 3em}"
            ".w,.i{background:#aaa;min-height:3em}"
            "</style>"
            "<form action=/restart method=post>"
        ));
        http.sendContent(F("<input type=submit value=\""));
        http.sendContent(_WSL_T.button_restart);
        http.sendContent(F("\"></form><hr><h1>"));
        http.sendContent(_WSL_T.title);
        http.sendContent(F("</h1><form method=post><label>"));
        http.sendContent(_WSL_T.ssid);
        http.sendContent(F(":<br><b class=s>"));
        http.sendContent(_WSL_T.scanning_long);
        http.sendContent("</b>");

        // Don't waste time scanning in captive portal detection (Apple)
        if (interactive) {
            if (num_networks < 0) num_networks = WiFi.scanNetworks();
            Serial.print(num_networks, DEC);
            Serial.println(F(" WiFi networks found."));
        }

        http.sendContent(F(
            "<style>.s{display:none}</style>"   // hide "scanning"
            "<select name=ssid onchange=\"document.getElementsByName('password')[0].value=''\">"
        ));

        String current = slurp("/wifi-ssid");
        bool found = false;
        for (int i = 0; i < num_networks; i++) {
            String opt = F("<option value='{ssid}'{sel}>{ssid} {lock} {1x}</option>");
            String ssid = WiFi.SSID(i);
            wifi_auth_mode_t mode = WiFi.encryptionType(i);

            opt.replace("{sel}",  ssid == current && !found ? " selected" : "");
            opt.replace("{ssid}", html_entities(ssid));
            opt.replace("{lock}", mode != WIFI_AUTH_OPEN ? "&#x1f512;" : "");
            opt.replace("{1x}",   mode == WIFI_AUTH_WPA2_ENTERPRISE ? _WSL_T.dot1x : F(""));
            http.sendContent(opt);

            if (ssid == current) found = true;
        }
        if (!found && current.length()) {
            String opt = F("<option value='{ssid}' selected>{ssid} (&#x26a0; not in range)</option>");
            opt.replace("{ssid}", html_entities(current));
            http.sendContent(opt);
        }

        http.sendContent(F("</select></label> <a href=/rescan onclick=\"this.innerHTML='"));
        http.sendContent(_WSL_T.scanning_short);
        http.sendContent("';\">");
        http.sendContent(_WSL_T.rescan);
        http.sendContent(F("</a><p><label>"));

        http.sendContent(_WSL_T.wifi_password);
        http.sendContent(F(":<br><input name=password value='"));
        if (slurp("/wifi-password").length()) http.sendContent("##**##**##**");
        http.sendContent(F("'></label><hr>"));

        if (WiFiSettingsLanguage::multiple()) {
            http.sendContent(F("<label>")); 
            http.sendContent(_WSL_T.language); 
            http.sendContent(F(":<br><select name=language>"));

            for (auto& lang : WiFiSettingsLanguage::languages) {
                String opt = F("<option value='{code}'{sel}>{name}</option>");
                opt.replace("{code}", lang.first);
                opt.replace("{name}", lang.second);
                opt.replace("{sel}", language == lang.first ? " selected" : "");
                http.sendContent(opt);
            }
            http.sendContent(F("</select></label>"));
        }

        for (auto& p : params) {
            http.sendContent(p->html());
        }

        http.sendContent(F(
            "<p style='position:sticky;bottom:0;text-align:right'>"
            "<input type=submit value=\""
        ));
        http.sendContent(_WSL_T.button_save);
        http.sendContent(F("\"style='font-size:150%'></form>"));
    });

    http.on("/", HTTP_POST, [this, &http]() {
        bool ok = true;
        if (! spurt("/wifi-ssid", http.arg("ssid"))) ok = false;

        if (WiFiSettingsLanguage::multiple()) {
            if (! spurt("/WiFiSettings-language", http.arg("language"))) ok = false;
            // Don't update immediately, because there is currently
            // no mechanism for reloading param strings.
            //language = http.arg("language");
            //WiFiSettingsLanguage::select(T, language);
        }

        String pw = http.arg("password");
        if (pw != "##**##**##**") {
            if (! spurt("/wifi-password", pw)) ok = false;
        }

        for (auto& p : params) {
            p->set(http.arg(p->name));
            if (! p->store()) ok = false;
        }

        if (ok) {
            http.sendHeader("Location", "/");
            http.send(302, "text/plain", "ok");
            if (onConfigSaved) onConfigSaved();
        } else {
            // Could be missing SPIFFS.begin(), unformatted filesystem, or broken flash.
            http.send(500, "text/plain", _WSL_T.error_fs);
        }
    });

    http.on("/restart", HTTP_POST, [this, &http]() {
        http.send(200, "text/plain", _WSL_T.bye);
        if (onRestart) onRestart();
        ESP.restart();
    });

    http.on("/rescan", HTTP_GET, [this, &http, &num_networks]() {
        http.sendHeader("Location", "/");
        http.send(302, "text/plain", _WSL_T.wait);
        num_networks = WiFi.scanNetworks();
    });

    http.onNotFound([this, &http, &redirect]() {
        if (redirect()) return;
        http.send(404, "text/plain", "404");
    });

    http.begin();

    for (;;) {
        http.handleClient();
        dns.processNextRequest();
        if (onPortalWaitLoop) onPortalWaitLoop();
        esp_task_wdt_reset();
        delay(1);
    }
}

bool WiFiSettingsClass::connect(bool portal, int wait_seconds) {
    begin();

    WiFi.mode(WIFI_STA);

    String ssid = slurp("/wifi-ssid");
    String pw = slurp("/wifi-password");
    if (ssid.length() == 0) {
        Serial.println(F("First contact!\n"));
        this->portal();
    }

    Serial.print(F("Connecting to WiFi SSID "));
    Serial.print(ssid);
    if (onConnect) onConnect();

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);  // arduino-esp32 #2537
    WiFi.setHostname(hostname.c_str());
    WiFi.begin(ssid.c_str(), pw.c_str());

    unsigned long starttime = millis();
    while (WiFi.status() != WL_CONNECTED && (wait_seconds < 0 || (millis() - starttime) < (unsigned)wait_seconds * 1000)) {
        Serial.print(".");
        delay(onWaitLoop ? onWaitLoop() : 100);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F(" failed."));
        if (onFailure) onFailure();
        if (portal) this->portal();
        return false;
    }

    Serial.println(WiFi.localIP().toString());
    if (onSuccess) onSuccess();
    return true;
}

void WiFiSettingsClass::begin() {
    if (begun) return;
    begun = true;

    // These things can't go in the constructor because the constructor runs
    // before ESPFS.begin()

    String user_language = slurp("/WiFiSettings-language");
    user_language.trim();
    if (user_language.length() && WiFiSettingsLanguage::available(user_language)) {
        language = user_language;
    }
    WiFiSettingsLanguage::select(_WSL_T, language);  // can update language

    if (!secure) {
        secure = checkbox(
            F("WiFiSettings-secure"),
            false,
            _WSL_T.portal_wpa
        );
    }

    if (!password.length()) {
        password = string(
            F("WiFiSettings-password"),
            8, 63,
            "",
            _WSL_T.portal_password
        );
        if (password == "") {
            // With regular 'init' semantics, the password would be changed
            // all the time.
            password = pwgen();
            params.back()->set(password);
            params.back()->store();
        }
    }

    if (hostname.endsWith("-")) hostname += ESPMAC;
}

WiFiSettingsClass::WiFiSettingsClass() {
    #ifdef ESP32
        hostname = F("esp32-");
    #else
        hostname = F("esp8266-");
    #endif

    language = "en";
}

WiFiSettingsClass WiFiSettings;
