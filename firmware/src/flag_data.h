#pragma once

#include <Arduino.h>

// --- FLAG DIMENSIONS (MUST match the bitmaps) ---
#define FLAG_W 32
#define FLAG_H 20
#define FLAG_SIZE (FLAG_W * FLAG_H) // 640 words

// Structure for the lookup table
struct FlagEntry {
    const char* code; // 2-letter country code (e.g., "FR")
    const uint16_t *bitmap; // Pointer to the PROGMEM bitmap data
};

// --- EXTERNAL DECLARATIONS FOR FLAG DATA ---
// These are defined in flag_data.cpp and are accessible globally.
extern const FlagEntry FLAG_LOOKUP[];
extern const int NUM_FLAGS;

// Optional: Explicitly declare the bitmaps if needed, though FLAG_LOOKUP is the main interface
extern const uint16_t FLAG_FR_DATA[FLAG_SIZE] PROGMEM;
extern const uint16_t FLAG_DE_DATA[FLAG_SIZE] PROGMEM;
extern const uint16_t FLAG_IN_DATA[FLAG_SIZE] PROGMEM;
extern const uint16_t FLAG_NL_DATA[FLAG_SIZE] PROGMEM;
extern const uint16_t FLAG_US_DATA[FLAG_SIZE] PROGMEM;
extern const uint16_t FLAG_UK_DATA[FLAG_SIZE] PROGMEM;
