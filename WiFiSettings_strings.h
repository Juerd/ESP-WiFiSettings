#include <map>

namespace WiFiSettingsLanguage {

struct Texts {
    const __FlashStringHelper
        *title,
        *portal_wpa,
        *portal_password,
        *wait,
        *bye,
        *error_fs,
        *button_save,
        *button_restart,
        *scanning_short,
        *scanning_long,
        *rescan,
        *dot1x,
        *ssid,
        *wifi_password,
        *language
    ;
    const char
        *init
    ;
};

#if \
   !defined LANGUAGE_EN \
&& !defined LANGUAGE_NL \
&& !defined LANGUAGE_DE \
&& !defined LANGUAGE_PL \
&& !defined LANGUAGE_ID \
&& !defined LANGUAGE_FR
    #define LANGUAGE_ALL
#endif

std::map<const String, const String> languages {
// Ordered alphabetically
#if defined LANGUAGE_ID || defined LANGUAGE_ALL
    { "id", "Bahasa Indonesia" },
#endif
#if defined LANGUAGE_DE || defined LANGUAGE_ALL
    { "de", "Deutsch" },
#endif
#if defined LANGUAGE_EN || defined LANGUAGE_ALL
    { "en", "English" },
#endif
#if defined LANGUAGE_EN || defined LANGUAGE_ALL
    { "fr", "Français" },
#endif
#if defined LANGUAGE_NL || defined LANGUAGE_ALL
    { "nl", "Nederlands" },
#endif
#if defined LANGUAGE_PL || defined LANGUAGE_ALL
    { "pl", "Polski" },
#endif

};

bool available(const String& language) {
    return languages.count(language) == 1;
}

bool multiple() {
    return languages.size() > 1;
}

bool select(Texts& T, String& language) {
    if (! available(language)) {
        if (available("en")) language = "en";
        else language = languages.begin()->first;
    }

#if defined LANGUAGE_EN || defined LANGUAGE_ALL
    if (language == "en") {
        T.title = F("Configuration");
        T.portal_wpa = F("Protect the configuration portal with a WiFi password");
        T.portal_password = F("WiFi password for the configuration portal");
        T.init = "default";
        T.wait = F("Wait for it...");
        T.bye = F("Bye!");
        T.error_fs = F("Error while writing to flash filesystem.");
        T.button_save = F("Save");
        T.button_restart = F("Restart device");
        T.scanning_short = F("Scanning...");
        T.scanning_long = F("Scanning for WiFi networks...");
        T.rescan = F("rescan");
        T.dot1x = F("(won't work: 802.1x is not supported)");
        T.ssid = F("WiFi network name (SSID)");
        T.wifi_password = F("WiFi password");
        T.language = F("Language");
        return true;
    }
#endif

#if defined LANGUAGE_NL || defined LANGUAGE_ALL
    if (language == "nl") {
        T.title = F("Configuratie");
        T.portal_wpa = F("Beveilig de configuratieportal met een WiFi-wachtwoord");
        T.portal_password = F("WiFi-wachtwoord voor de configuratieportal");
        T.init = "standaard";
        T.wait = F("Even wachten...");
        T.bye = F("Doei!");
        T.error_fs = F("Fout bij het schrijven naar het flash-bestandssysteem.");
        T.button_save = F("Opslaan");
        T.button_restart = F("Herstarten");
        T.scanning_short = F("Scant...");
        T.scanning_long = F("Zoeken naar WiFi-netwerken...");
        T.rescan = F("opnieuw scannen");
        T.dot1x = F("(werkt niet: 802.1x wordt niet ondersteund)");
        T.ssid = F("WiFi-netwerknaam (SSID)");
        T.wifi_password = F("WiFi-wachtwoord");
        T.language = F("Taal");
        return true;
    }
#endif

#if defined LANGUAGE_DE || defined LANGUAGE_ALL
    if (language == "de") {
        T.title = F("Konfiguration");
        T.portal_wpa = F("Das Konfigurationsportal mit einem Passwort schützen");
        T.portal_password = F("Passwort für das Konfigurationsportal");
        T.init = "Standard";
        T.wait = F("Warten...");
        T.bye = F("Tschüss!");
        T.error_fs = F("Fehler beim Schreiben auf das Flash-Dateisystem");
        T.button_save = F("Speichern");
        T.button_restart = F("Gerät neustarten");
        T.scanning_short = F("Suchen...");
        T.scanning_long = F("Suche nach WiFi-Netzwerken...");
        T.rescan = F("Erneut suchen");
        T.dot1x = F("(nicht möglich: 802.1x nicht unterstützt)");
        T.ssid = F("WiFi Netzwerkname (SSID)");
        T.wifi_password = F("WiFi Passwort");
        T.language = F("Sprache");
        return true;
    }
#endif

#if defined LANGUAGE_PL || defined LANGUAGE_ALL
    if (language == "pl") {
        T.title = F("Konfiguracja");
        T.portal_wpa = F("Zabezpiecz portal konfiguracyjny hasłem WiFi");
        T.portal_password = F("Hasło WiFi dla portalu konfiguracyjnego");
        T.init = "domyślne";
        T.wait = F("Poczekaj...");
        T.bye = F("Do zobaczenia!");
        T.error_fs = F("Błąd podczas zapisu do pamięci flash.");
        T.button_save = F("Zapisz");
        T.button_restart = F("Uruchom ponownie");
        T.scanning_short = F("Skanowanie...");
        T.scanning_long = F("Skanowanie sieci WiFi...");
        T.rescan = F("skanuj ponownie");
        T.dot1x = F("(nie będzie działać: 802.1x nie jest obsługiwane)");
        T.ssid = F("Nazwa sieci WiFi (SSID)");
        T.wifi_password = F("Hasło WiFi");
        T.language = F("Język");
        return true;
    }
#endif

#if defined LANGUAGE_ID || defined LANGUAGE_ALL
    if (WiFiSettings.language == "id") {
       T.title = F("Konfigurasi");
       T.portal_wpa = F("Lindungi portal konfigurasi dengan kata sandi WiFi");
       T.portal_password = F("Kata sandi WiFi untuk portal konfigurasi");
       T.init = "default";
       T.wait = F("Tunggu...");
       T.bye = F("Selamat tinggal!");
       T.error_fs = F("Terjadi kesalahan saat menulis ke sistem berkas flash.");
       T.button_save = F("Simpan");
       T.button_restart = F("Mulai ulang perangkat");
       T.scanning_short = F("Memindai...");
       T.scanning_long = F("Memindai jaringan WiFi...");
       T.rescan = F("memindai ulang");
       T.dot1x = F("(tidak berfungsi: 802.1x tidak didukung)");
       T.ssid = F("Nama jaringan WiFi (SSID)");
       T.wifi_password = F("Kata sandi WiFi"); 
       T.language = F("Bahasa");
       return true;
    }
#endif

#if defined LANGUAGE_FR || defined LANGUAGE_ALL
    if (language == "fr") {
       T.title = F("Configuration");
       T.portal_wpa = F("Protéger le portail de configuration avec un mot de passe");
       T.portal_password = F("Mot de passe pour le portail de configuration");
       T.init = "par défaut";
       T.wait = F("Un moment...");
       T.bye = F("Au revoir!");
       T.error_fs = F("Erreur d'écriture dans la mémoire flash.");
       T.button_save = F("Enregistrer");
       T.button_restart = F("Redémarrer");
       T.scanning_short = F("Recherche...");
       T.scanning_long = F("Recherche les réseaux WiFi...");
       T.rescan = F("nouvelle recherche");
       T.dot1x = F("(802.1x n'est pas supporté)");
       T.ssid = F("Nom du résau WiFi (SSID)");
       T.wifi_password = F("Mot de passe WiFi");
       T.language = F("Langue");
       return true;
    }
#endif

    return false;
}

} // namespace
