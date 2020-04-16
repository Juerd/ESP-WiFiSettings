#include <WiFiConfig.h>
#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <HTTP_Method.h>
#include <esp_task_wdt.h>
#include <limits.h>
#include <vector>

#define Sprintf(f, ...) ({ char* s; asprintf(&s, f, __VA_ARGS__); String r = s; free(s); r; })

String slurp(String fn) {
    File f = SPIFFS.open(fn, "r");
    String r = f.readString();
    f.close();
    return r;
}

void spurt(String fn, String content) {
    File f = SPIFFS.open(fn, "w");
    f.print(content);
    f.close();
}

String pwgen() {
    const char* filename     = "/my-password";
    const char* passchars    = "ABCEFGHJKLMNPRSTUXYZabcdefhkmnorstvxz23456789-#@%^";

    String password = slurp(filename);

    if (password.length() == 0) {
        for (int i = 0; i < 16; i++) {
             password.concat( passchars[random(strlen(passchars))] );
        }
        spurt(filename, password);
    }

    return password;
}
String html_entities(String raw) {
    String r;
    for (int i = 0; i < raw.length(); i++) {
        char c = raw.charAt(i);
        if (c >= '!' && c <= 'z' && c != '&' && c != '<' && c != '>') {
            // printable ascii minus html and {}
            r += c;
        } else {
            r += Sprintf("&#%d;", raw.charAt(i));
        }
    }
    return r;
}

struct WiFiConfigParameter {
    String name;
    String label;
    String value;
    String init;
    long min = LONG_MIN;
    long max = LONG_MAX;

    String filename() { String fn = "/"; fn += name; return fn; }
    void store(String v) { value = v; spurt(filename(), v); }
    void fill() { value = slurp(filename()); }
    virtual String html() = 0;
};

struct WiFiConfigString : WiFiConfigParameter {
    String html() {
        String h = "<label>{label}:<br><input name='{name}' value='{value}' placeholder='{init}' maxlength={max}></label>";
        h.replace("{name}", html_entities(name));
        h.replace("{value}", html_entities(value));
        h.replace("{init}", html_entities(init));
        h.replace("{label}", html_entities(label));
        h.replace("{max}", String(max));
        return h;
    }
};

struct WiFiConfigInt : WiFiConfigParameter {
    String html() {
        String h = "<label>{label}:<br><input type=number step=1 min={min} max={max} name='{name}' value='{value}' placeholder='{init}'></label>";
        h.replace("{name}", html_entities(name));
        h.replace("{value}", html_entities(value));
        h.replace("{init}", html_entities(init));
        h.replace("{label}", html_entities(label));
        h.replace("{min}", String(min));
        h.replace("{max}", String(max));
        return h;
    }
};

struct std::vector<WiFiConfigParameter*> params;

String WiFiConfigClass::string(String name, String init, String label) {
    struct WiFiConfigString* x = new WiFiConfigString();
    x->name = name;
    x->label = label.length() ? label : name;
    x->init = init;
    x->fill();

    params.push_back(x);
    return x->value.length() ? x->value : x->init;
}

String WiFiConfigClass::string(String name, unsigned int max_length, String init, String label) {
    String rv = string(name, init, label);
    params.back()->max = max_length;
    return rv;
}

long WiFiConfigClass::integer(String name,  long init, String label) {
    struct WiFiConfigString* x = new WiFiConfigString();
    x->name = name;
    x->label = label.length() ? label : name;
    x->init = init;
    x->fill();

    params.push_back(x);
    return (x->value.length() ? x->value : x->init).toInt();
}

long WiFiConfigClass::integer(String name, long min, long max,  long init, String label) {
    long rv = integer(name, init, label);
    params.back()->min = min;
    params.back()->max = max;
    return rv;
}

void WiFiConfigClass::portal() {
    static WebServer http(80);
    static DNSServer dns;
    static int num_networks = -1;
    String wpa = slurp("/wifi-portal-wpa");
    String my_password = pwgen();

    if (onPortal) onPortal();
    WiFi.disconnect(true, true);    // reset state so .scanNetworks() works

    Serial.println("Starting access point for configuration portal.");
    if (wpa.length() && my_password.length()) {
        Serial.printf("SSID: '%s', Password: '%s'\n", hostname.c_str(), my_password.c_str());
        WiFi.softAP(hostname.c_str(), my_password.c_str());
    } else {
        Serial.printf("SSID: '%s'\n", hostname.c_str());
        WiFi.softAP(hostname.c_str());
    }
    delay(500);
    dns.setTTL(0);
    dns.start(53, "*", WiFi.softAPIP());

    Serial.println(WiFi.softAPIP().toString());

    http.on("/", HTTP_GET, [this]() {
        String html = "<!DOCTYPE html>\n<meta charset=UTF-8>"
            "<title>{hostname}</title>"
            "<form action=/restart method=post>"
                "Hello, my name is {hostname}."
                "<p>Currently configured SSID: {ssid}<br>"
                "<input type=submit value='Restart device'>"
            "</form>"
            "<hr>"
            "<h2>Configureren</h2>"
            "<form method=post>"
                "SSID: <select name=ssid onchange=\"document.getElementsByName('password')[0].value=''\">{options}</select> "
                "<a href=/rescan onclick=\"this.innerHTML='scanning...';\">rescan</a>"
                "</select><br>WiFi WEP/WPA password: <input name=password value='{password}'><br>"
                "<p>My password: <input name=my_password value='{my_password}' minlength=8 required> (8+ characters)<br>"
                "<label><input type=checkbox name=portalpw value=yes{portalwpa}> Require &uarr;password&uarr; for configuration portal</label><hr>";

        for (auto p : params) html += p->html() + "<p>";

        html +=
                "<input type=submit value=Save>"
            "</form>";

        String current = slurp("/wifi-ssid");
        String pw = slurp("/wifi-password");

        html.replace("{hostname}",    hostname);
        html.replace("{ssid}",        current.length() ? html_entities(current) : "(not set)");
        html.replace("{portalwpa}",   slurp("/wifi-portal-wpa").length() ? " checked" : "");
        html.replace("{my_password}", html_entities(pwgen()));

        String options;
        if (num_networks < 0) num_networks = WiFi.scanNetworks();
        Serial.printf("%d WiFi networks found.\n", num_networks);

        bool found = false;
        for (int i = 0; i < num_networks; i++) {
            String opt = "<option value='{ssid}'{sel}>{ssid} {lock} {1x}</option>";
            String ssid = WiFi.SSID(i);
            wifi_auth_mode_t mode = WiFi.encryptionType(i);

            opt.replace("{sel}",  ssid == current && !(found++) ? " selected" : "");
            opt.replace("{ssid}", html_entities(ssid));
            opt.replace("{lock}", mode != WIFI_AUTH_OPEN ? "&#x1f512;" : "");
            opt.replace("{1x}",   mode == WIFI_AUTH_WPA2_ENTERPRISE ? "(won't work: 802.1x is not supported)" : "");
            options += opt;
        }
        html.replace("{password}", found && pw.length() ? "##**##**##**" : "");
        html.replace("{options}",    options);
        http.send(200, "text/html", html);
    });

    http.on("/", HTTP_POST, [this]() {
        String pw = http.arg("password");
        if (pw != "##**##**##**") {
            spurt("/wifi-password", pw);
        }
        spurt("/wifi-ssid",       http.arg("ssid"));
        spurt("/wifi-portal-wpa", http.arg("portalpw") == "yes" ? "x" : "");
        spurt("/my-password",     http.arg("my_password"));
        for (auto p : params) p->store(http.arg(p->name));
        http.sendHeader("Location", "/");
        http.send(302, "text/plain", "ok");
        if (onConfigSaved) onConfigSaved();
    });

    http.on("/restart", HTTP_POST, [this]() {
        http.send(200, "text/plain", "Doei!");
        if (onRestart) onRestart();
        ESP.restart();
    });

    http.on("/rescan", HTTP_GET, [this]() {
        http.sendHeader("Location", "/");
        http.send(302, "text/plain", "wait for it...");
        num_networks = WiFi.scanNetworks();
    });

    http.onNotFound([this]() {
        http.sendHeader("Location", "http://" + hostname + "/");
        http.send(302, "text/plain", "hoi");
    });

    http.begin();

    for (;;) {
        http.handleClient();
        dns.processNextRequest();
        if (onPortalWaitLoop) onPortalWaitLoop();
        esp_task_wdt_reset();
        vTaskDelay(1);
    }
}

bool WiFiConfigClass::connect(bool portal, int wait_seconds) {
    String ssid = slurp("/wifi-ssid");
    String pw = slurp("/wifi-password");
    if (ssid.length() == 0 && portal) {
        Serial.println("First contact!\n");
        this->portal();
    }

    Serial.printf("Connecting to WiFi %s", ssid.c_str());
    if (onConnect) onConnect();
    WiFi.begin(ssid.c_str(), pw.c_str());

    unsigned long starttime = millis();
    while (WiFi.status() != WL_CONNECTED && (wait_seconds < 0 || (millis() - starttime) < wait_seconds * 1000)) {
        Serial.print(".");
        delay(onWaitLoop ? onWaitLoop() : 100);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(" failed.");
        if (onFailure) onFailure();
        if (portal) this->portal();
        return false;
    }

    Serial.println(WiFi.softAPIP().toString());
    if (onFailure) onSuccess();
    return true;
}

WiFiConfigClass::WiFiConfigClass() {
    hostname = Sprintf("esp32-%06" PRIx64, ESP.getEfuseMac() >> 24);
}

WiFiConfigClass WiFiConfig;
