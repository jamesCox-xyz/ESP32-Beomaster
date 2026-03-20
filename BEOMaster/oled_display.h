// =============================================================================
// oled_display.h — 0.96" I2C OLED status display (SSD1306 128×64)
// =============================================================================
// Shows a one-word status on the top half and the device IP on the bottom.
//
// Required libraries (install via Library Manager):
//   • Adafruit SSD1306
//   • Adafruit GFX Library
//
// Default I2C pins on ESP32:  SDA = GPIO 21,  SCL = GPIO 22
// I2C address: 0x3C (most common for 0.96" modules)
// =============================================================================

#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- Configuration ----------------------------------------------------

#define OLED_WIDTH    128
#define OLED_HEIGHT    64
#define OLED_RESET     -1      // -1 = share ESP reset pin
#define OLED_I2C_ADDR 0x3C

// How long (ms) "IR TX" is shown before reverting to the previous status
#define IR_TX_DISPLAY_MS 1000

// ---------- Internal state ---------------------------------------------------

static Adafruit_SSD1306 _oled(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

static String  _oled_status  = "";
static String  _oled_ip      = "";
static bool    _oled_ir_showing = false;
static uint32_t _oled_ir_ts  = 0;
static bool    _oled_ready   = false;

// ---------- Internal draw helper ---------------------------------------------

static void _oled_draw() {
    _oled.clearDisplay();

    // ---- Status label (large, centred vertically in top half) ---------------
    _oled.setTextSize(3);
    _oled.setTextColor(SSD1306_WHITE);

    // Measure text to centre horizontally (each char is 18px wide at size 3)
    int16_t  x1, y1;
    uint16_t tw, th;
    _oled.getTextBounds(_oled_status.c_str(), 0, 0, &x1, &y1, &tw, &th);
    int xPos = (OLED_WIDTH - (int)tw) / 2;
    if (xPos < 0) xPos = 0;

    _oled.setCursor(xPos, 4);
    _oled.print(_oled_status);

    // ---- Divider line -------------------------------------------------------
    _oled.drawFastHLine(0, 38, OLED_WIDTH, SSD1306_WHITE);

    // ---- IP address (small, centred in bottom quarter) ----------------------
    _oled.setTextSize(1);
    _oled.getTextBounds(_oled_ip.c_str(), 0, 0, &x1, &y1, &tw, &th);
    xPos = (OLED_WIDTH - (int)tw) / 2;
    if (xPos < 0) xPos = 0;
    _oled.setCursor(xPos, 47);
    _oled.print(_oled_ip);

    _oled.display();
}

// ---------- Public API -------------------------------------------------------

// Call once in setup() — returns false if display not detected
inline bool oled_init() {
    if (!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        Serial.println("[OLED] Display not found — check wiring / I2C address.");
        return false;
    }
    _oled.clearDisplay();
    _oled.display();
    _oled_ready = true;
    return true;
}

// Set the persistent status and IP, then redraw
inline void oled_set_status(const char* status, const char* ip = "") {
    if (!_oled_ready) return;
    _oled_status = status;
    _oled_ip     = ip;
    if (!_oled_ir_showing) {   // don't overwrite an active IR TX flash
        _oled_draw();
    }
}

// Flash "IR TX" for IR_TX_DISPLAY_MS ms, then revert automatically via oled_loop()
inline void oled_show_ir_tx() {
    if (!_oled_ready) return;
    _oled_ir_showing = true;
    _oled_ir_ts      = millis();
    String saved_status = _oled_status;
    _oled_status = "IR TX";
    _oled_draw();
    _oled_status = saved_status;   // restore so revert works correctly
}

// Call every loop() iteration to handle the IR TX timeout
inline void oled_loop() {
    if (!_oled_ready || !_oled_ir_showing) return;
    if (millis() - _oled_ir_ts >= IR_TX_DISPLAY_MS) {
        _oled_ir_showing = false;
        _oled_draw();
    }
}
