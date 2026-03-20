// =============================================================================
// BEOMaster 5500 — ESP32 IR Web Controller
// =============================================================================
// Hosts a Wi-Fi web page with all MCP 5500 control buttons.
// Pressing a button sends a GET request to /cmd?code=<n> which fires the
// corresponding B&O Legacy IR command via the IR transmitter circuit.
//
// Files:
//   BEOMaster.ino    — this file; Wi-Fi setup and HTTP routing
//   beo_commands.h   — BEO_CMD_* constant definitions
//   ir_transmitter.h — IR encoding and LEDC carrier generation
//   web_ui.h         — full HTML page stored in flash (PROGMEM)
//   wifi_setup_ui.h  — captive portal HTML page for WiFi setup
//
// First-time setup:
//   1. Set WIFI_SSID and WIFI_PASSWORD in credentials.h.
//   2. Upload with IR_CIRCUIT_READY 0 (ir_transmitter.h) to test the web UI.
//   3. Build the IR circuit (see BEOMASTER_5500_Project.md).
//   4. Set IR_CIRCUIT_READY 1, re-upload, and verify commands with the Beomaster.
// =============================================================================

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include "credentials.h"   // Copy credentials.h.example → credentials.h and fill in your details
#include "beo_commands.h"
#include "ir_transmitter.h"
#include "web_ui.h"
#include "wifi_setup_ui.h"
#include "settings_ui.h"
#include "oled_display.h"

// Wi-Fi credentials come from credentials.h as WIFI_SSID / WIFI_PASSWORD macros

// ---------- Button code table ------------------------------------------------

struct ButtonCode {
    const char* key;
    uint8_t     defaultCode;
    uint8_t     code;
};

static ButtonCode buttons[] = {
    { "standby",      0x0C, 0x0C },
    { "volume_up",    0x60, 0x60 },
    { "volume_down",  0x64, 0x64 },
    { "mute",         0x0D, 0x0D },
    { "loudness",     0x3C, 0x3C },
    { "bass_up",      0x70, 0x70 },
    { "bass_down",    0x74, 0x74 },
    { "treble_up",    0x78, 0x78 },
    { "treble_down",  0x7C, 0x7C },
    { "bal_left",     0x68, 0x68 },
    { "bal_right",    0x6C, 0x6C },
    { "fm",           0x81, 0x81 },
    { "am",           0x82, 0x82 },
    { "phono",        0x83, 0x83 },
    { "cd",           0x92, 0x92 },
    { "tape1",        0x87, 0x87 },
    { "tape2",        0x88, 0x88 },
    { "aux",          0x8A, 0x8A },
    { "preset_up",    0x1E, 0x1E },
    { "preset_down",  0x1F, 0x1F },
    { "tune_up",      0x1C, 0x1C },
    { "tune_down",    0x1D, 0x1D },
    { "store_preset", 0x5C, 0x5C },
    { "num_0",        0x20, 0x20 },
    { "num_1",        0x21, 0x21 },
    { "num_2",        0x22, 0x22 },
    { "num_3",        0x23, 0x23 },
    { "num_4",        0x24, 0x24 },
    { "num_5",        0x25, 0x25 },
    { "num_6",        0x26, 0x26 },
    { "num_7",        0x27, 0x27 },
    { "num_8",        0x28, 0x28 },
    { "num_9",        0x29, 0x29 },
    { "play",         0x35, 0x35 },
    { "stop",         0x36, 0x36 },
    { "record",       0x37, 0x37 },
    { "timer",        0x44, 0x44 },
    { "clock",        0x43, 0x43 },
    { "sleep",        0x45, 0x45 },
};
const int NUM_BUTTONS = sizeof(buttons) / sizeof(buttons[0]);

// ---------- Global State -----------------------------------------------------

const byte DNS_PORT = 53;
DNSServer dnsServer;
Preferences preferences;
bool apMode = false;

void loadButtonCodes() {
    preferences.begin("beo-codes", true);
    for (int i = 0; i < NUM_BUTTONS; i++) {
        buttons[i].code = preferences.getUChar(buttons[i].key, buttons[i].defaultCode);
    }
    preferences.end();
}

// ---------- HTTP server on port 80 -------------------------------------------
WebServer server(80);

// ---------- Route handlers ---------------------------------------------------

void handleRoot() {
    server.send_P(200, "text/html", WEB_UI_HTML);
}

void handleCommand() {
    if (!server.hasArg("code")) {
        server.send(400, "text/plain", "Missing ?code= parameter");
        return;
    }

    long raw = server.arg("code").toInt();
    if (raw < 0 || raw > 255) {
        server.send(400, "text/plain", "Code out of range (0-255)");
        return;
    }

    uint8_t code = (uint8_t)raw;
    ir_send_beo(code);
    oled_show_ir_tx();

    server.send(200, "text/plain", "OK");
}

void handleWifiSetup() {
    server.send_P(200, "text/html", WIFI_SETUP_HTML);
}

void handleWifiSave() {
    if (server.hasArg("ssid")) {
        String newSSID = server.arg("ssid");
        String newPass = server.hasArg("pass") ? server.arg("pass") : "";
        
        preferences.begin("beo-wifi", false);
        preferences.putString("ssid", newSSID);
        preferences.putString("pass", newPass);
        preferences.end();
        
        String html = "<html><body style='background:#111;color:#c8a96e;font-family:sans-serif;text-align:center;padding:50px;'>";
        html += "<h2>Credentials saved!</h2><p>Restarting ESP32... Please connect back to your normal WiFi network.</p></body></html>";
        server.send(200, "text/html", html);
        
        delay(2000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "SSID is required.");
    }
}

void handleCaptivePortalRedirect() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
}

void handleSettings() {
    server.send_P(200, "text/html", SETTINGS_HTML);
}

void handleCodes() {
    String json = "{";
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (i > 0) json += ",";
        json += "\"";
        json += buttons[i].key;
        json += "\":";
        json += buttons[i].code;
    }
    json += "}";
    server.send(200, "application/json", json);
}

void handleSaveSettings() {
    preferences.begin("beo-codes", false);
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (server.hasArg(buttons[i].key)) {
            long val = server.arg(buttons[i].key).toInt();
            if (val >= 0 && val <= 255) {
                buttons[i].code = (uint8_t)val;
                preferences.putUChar(buttons[i].key, buttons[i].code);
            }
        }
    }
    preferences.end();
    server.send(200, "text/plain", "OK");
}

void handleNotFound() {
    server.send(404, "text/plain", "Not found");
}

// ---------- setup / loop -----------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== Beomaster 5500 IR Controller ===");

    // Initialise OLED display
    oled_init();
    oled_set_status("BOOT", "connecting...");

    // Load saved button codes from flash (falls back to defaults)
    loadButtonCodes();

    // Initialise IR transmitter (stub or real depending on IR_CIRCUIT_READY)
    ir_init();

    // Read stored credentials or fallback
    preferences.begin("beo-wifi", true);
    String ssid = preferences.getString("ssid", WIFI_SSID);
    String pass = preferences.getString("pass", WIFI_PASSWORD);
    preferences.end();

    // Connect to Wi-Fi
    Serial.printf("[WiFi] Connecting to %s", ssid.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\n[WiFi] Failed to connect. Falling back to AP mode.");
        apMode = true;

        WiFi.mode(WIFI_AP);
        WiFi.softAP("Beomaster Setup");
        delay(500);

        // Setup captive portal
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

        server.on("/", HTTP_GET, handleWifiSetup);
        server.on("/save_wifi", HTTP_POST, handleWifiSave);
        server.onNotFound(handleCaptivePortalRedirect);

        oled_set_status("ERROR", WiFi.softAPIP().toString().c_str());
        Serial.printf("[WiFi] AP Mode started. Connect to 'Beomaster Setup' and visit http://%s/\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println();
        Serial.printf("[WiFi] Connected to %s. IP: %s\n", ssid.c_str(), WiFi.localIP().toString().c_str());

        if (MDNS.begin("beomasterESP")) {
            Serial.println("[mDNS] Responder started. Accessible at http://beomasterESP.local/");
        }

        oled_set_status("ACTIVE", WiFi.localIP().toString().c_str());

        // Register HTTP routes
        server.on("/",              HTTP_GET,  handleRoot);
        server.on("/cmd",           HTTP_GET,  handleCommand);
        server.on("/codes",         HTTP_GET,  handleCodes);
        server.on("/settings",      HTTP_GET,  handleSettings);
        server.on("/save_settings", HTTP_POST, handleSaveSettings);
        server.onNotFound(handleNotFound);
    }

    server.begin();
    Serial.println("[HTTP] Server started on port 80.");
}

void loop() {
    if (apMode) {
        dnsServer.processNextRequest();
    }
    server.handleClient();
    oled_loop();
}
