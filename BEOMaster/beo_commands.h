#pragma once
#include <stdint.h>

// =============================================================================
// Bang & Olufsen Beomaster 5500 — IR Command Codes
// =============================================================================
// Protocol: B&O Legacy (pre-Beo4), 40 kHz carrier, 8-bit commands
//
// NOTE: These codes are based on known B&O IR databases (LIRC) and community
// documentation. Verify each code against your physical remote before finalising.
// Use an IR receiver module (e.g. TSOP38238) and Serial output to learn/confirm
// any codes that don't work as expected.
// =============================================================================

// --- Power / Standby ---------------------------------------------------------
constexpr uint8_t BEO_CMD_STANDBY        = 0x0C;

// --- Volume ------------------------------------------------------------------
constexpr uint8_t BEO_CMD_VOLUME_UP      = 0x60;
constexpr uint8_t BEO_CMD_VOLUME_DOWN    = 0x64;
constexpr uint8_t BEO_CMD_MUTE           = 0x0D;
constexpr uint8_t BEO_CMD_LOUDNESS       = 0x3C;

// --- Source Selection --------------------------------------------------------
constexpr uint8_t BEO_CMD_FM             = 0x81;   // FM tuner
constexpr uint8_t BEO_CMD_AM             = 0x82;   // AM tuner
constexpr uint8_t BEO_CMD_PHONO         = 0x83;   // Phono / turntable
constexpr uint8_t BEO_CMD_CD            = 0x92;   // CD player
constexpr uint8_t BEO_CMD_TAPE1         = 0x87;   // Tape deck 1
constexpr uint8_t BEO_CMD_TAPE2         = 0x88;   // Tape deck 2
constexpr uint8_t BEO_CMD_AUX           = 0x8A;   // Auxiliary / Video

// --- Tuner / Preset ----------------------------------------------------------
constexpr uint8_t BEO_CMD_PRESET_UP     = 0x1E;   // Next preset station
constexpr uint8_t BEO_CMD_PRESET_DOWN   = 0x1F;   // Previous preset station
constexpr uint8_t BEO_CMD_TUNE_UP       = 0x1C;   // Manual tune up
constexpr uint8_t BEO_CMD_TUNE_DOWN     = 0x1D;   // Manual tune down
constexpr uint8_t BEO_CMD_STORE        = 0x5C;   // Store/save preset

// --- Tone Controls -----------------------------------------------------------
// NOTE: Verify these — tone control codes vary between Beomaster models
constexpr uint8_t BEO_CMD_BASS_UP       = 0x70;
constexpr uint8_t BEO_CMD_BASS_DOWN     = 0x74;
constexpr uint8_t BEO_CMD_TREBLE_UP     = 0x78;
constexpr uint8_t BEO_CMD_TREBLE_DOWN   = 0x7C;

// --- Balance -----------------------------------------------------------------
constexpr uint8_t BEO_CMD_BALANCE_LEFT  = 0x68;
constexpr uint8_t BEO_CMD_BALANCE_RIGHT = 0x6C;

// --- Timer / Clock -----------------------------------------------------------
constexpr uint8_t BEO_CMD_TIMER        = 0x44;
constexpr uint8_t BEO_CMD_CLOCK        = 0x43;
constexpr uint8_t BEO_CMD_SLEEP        = 0x45;

// --- Tape Functions ----------------------------------------------------------
constexpr uint8_t BEO_CMD_RECORD       = 0x37;
constexpr uint8_t BEO_CMD_PLAY         = 0x35;
constexpr uint8_t BEO_CMD_STOP         = 0x36;

// --- Numeric (for preset entry) ----------------------------------------------
constexpr uint8_t BEO_CMD_NUM_0        = 0x20;
constexpr uint8_t BEO_CMD_NUM_1        = 0x21;
constexpr uint8_t BEO_CMD_NUM_2        = 0x22;
constexpr uint8_t BEO_CMD_NUM_3        = 0x23;
constexpr uint8_t BEO_CMD_NUM_4        = 0x24;
constexpr uint8_t BEO_CMD_NUM_5        = 0x25;
constexpr uint8_t BEO_CMD_NUM_6        = 0x26;
constexpr uint8_t BEO_CMD_NUM_7        = 0x27;
constexpr uint8_t BEO_CMD_NUM_8        = 0x28;
constexpr uint8_t BEO_CMD_NUM_9        = 0x29;
