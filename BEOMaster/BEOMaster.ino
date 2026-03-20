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
#include "oled_display.h"

// Wi-Fi credentials come from credentials.h as WIFI_SSID / WIFI_PASSWORD macros

// ---------- Global State -----------------------------------------------------

const byte DNS_PORT = 53;
DNSServer dnsServer;
Preferences preferences;
bool apMode = false;

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
        server.on("/",    HTTP_GET, handleRoot);
        server.on("/cmd", HTTP_GET, handleCommand);
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
