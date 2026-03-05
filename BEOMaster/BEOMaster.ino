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
//
// First-time setup:
//   1. Set WIFI_SSID and WIFI_PASSWORD below.
//   2. Upload with IR_CIRCUIT_READY 0 (ir_transmitter.h) to test the web UI.
//   3. Build the IR circuit (see BEOMASTER_5500_Project.md).
//   4. Set IR_CIRCUIT_READY 1, re-upload, and verify commands with the Beomaster.
// =============================================================================

#include <WiFi.h>
#include <WebServer.h>
#include "credentials.h"   // Copy credentials.h.example → credentials.h and fill in your details
#include "beo_commands.h"
#include "ir_transmitter.h"
#include "web_ui.h"

// Wi-Fi credentials come from credentials.h as WIFI_SSID / WIFI_PASSWORD macros

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

    // Initialise IR transmitter (stub or real depending on IR_CIRCUIT_READY)
    ir_init();

    // Connect to Wi-Fi
    Serial.printf("Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\n[WiFi] Failed to connect. Check credentials and reset.");
    } else {
        Serial.println();
        Serial.printf("[WiFi] Connected. Open http://%s/ in a browser.\n",
                      WiFi.localIP().toString().c_str());
    }

    // Register HTTP routes
    server.on("/",    HTTP_GET, handleRoot);
    server.on("/cmd", HTTP_GET, handleCommand);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("[HTTP] Server started on port 80.");
}

void loop() {
    server.handleClient();
}
