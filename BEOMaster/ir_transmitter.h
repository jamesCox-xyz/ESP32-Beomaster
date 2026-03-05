#pragma once
#include <Arduino.h>

// =============================================================================
// IR Transmitter — B&O Legacy Protocol
// =============================================================================
// Carrier frequency : 40 kHz (period ~25 µs)
// Encoding          : Space-encoded (pulse-distance)
// Header            : 200 µs mark, 3125 µs space
// Bit '0'           : 200 µs mark, 3125 µs space
// Bit '1'           : 200 µs mark, 6250 µs space
// Frame gap         : >= 25 ms silence between frames
// Command length    : 8 bits, MSB first
//
// Hardware: Connect IR LED (+ series resistor) to IR_TX_PIN via a transistor
//           driver (e.g. 2N2222 / BC547). See project .md for full schematic.
//
// When IR_CIRCUIT_READY is 0 the transmit functions log to Serial only.
// Set to 1 once the hardware circuit is assembled and wired.
// =============================================================================

// ---------- Configuration ----------------------------------------------------

#define IR_TX_PIN        4       // GPIO pin to the IR LED transistor base driver
#define IR_LEDC_CHANNEL  0       // ESP32 LEDC channel (0–15)
#define IR_CIRCUIT_READY 0       // Set to 1 when hardware is connected

// ---------- Protocol timing (µs) ---------------------------------------------

#define BEO_PULSE_US       200
#define BEO_ZERO_SPACE_US  3125
#define BEO_ONE_SPACE_US   6250
#define BEO_HEADER_SPACE_US 3125
#define BEO_GAP_US         25000   // inter-frame gap

// ---------- Low-level helpers ------------------------------------------------

static inline void ir_carrier_on() {
#if IR_CIRCUIT_READY
    ledcWrite(IR_LEDC_CHANNEL, 128);   // ~50 % duty cycle on 40 kHz carrier
#endif
}

static inline void ir_carrier_off() {
#if IR_CIRCUIT_READY
    ledcWrite(IR_LEDC_CHANNEL, 0);
#endif
}

static void ir_mark(uint16_t us) {
    ir_carrier_on();
    delayMicroseconds(us);
}

static void ir_space(uint16_t us) {
    ir_carrier_off();
    delayMicroseconds(us);
}

// ---------- Public API -------------------------------------------------------

/**
 * @brief Initialise the LEDC peripheral for 40 kHz IR carrier output.
 *        Call once from setup().
 */
void ir_init() {
#if IR_CIRCUIT_READY
    ledcSetup(IR_LEDC_CHANNEL, 40000, 8);   // 40 kHz, 8-bit resolution
    ledcAttachPin(IR_TX_PIN, IR_LEDC_CHANNEL);
    ledcWrite(IR_LEDC_CHANNEL, 0);          // Start with carrier off
    Serial.println("[IR] Hardware transmitter initialised on GPIO " + String(IR_TX_PIN));
#else
    Serial.println("[IR] STUB mode — IR_CIRCUIT_READY=0. Commands logged only.");
    Serial.println("[IR] Set IR_CIRCUIT_READY=1 in ir_transmitter.h when circuit is built.");
#endif
}

/**
 * @brief Transmit an 8-bit B&O legacy command.
 * @param command  One of the BEO_CMD_* constants from beo_commands.h
 * @param repeats  Number of times to send the frame (default 3 for reliability)
 */
void ir_send_beo(uint8_t command, uint8_t repeats = 3) {
    Serial.printf("[IR] Sending B&O command: 0x%02X (%s)\n", command,
                  IR_CIRCUIT_READY ? "TX" : "STUB");

    for (uint8_t r = 0; r < repeats; r++) {
        // Header pulse
        ir_mark(BEO_PULSE_US);
        ir_space(BEO_HEADER_SPACE_US);

        // 8 data bits, MSB first
        for (int8_t bit = 7; bit >= 0; bit--) {
            ir_mark(BEO_PULSE_US);
            if (command & (1 << bit)) {
                ir_space(BEO_ONE_SPACE_US);
            } else {
                ir_space(BEO_ZERO_SPACE_US);
            }
        }

        // Trailing pulse + inter-frame gap
        ir_mark(BEO_PULSE_US);
        ir_space(BEO_GAP_US);
    }
}
