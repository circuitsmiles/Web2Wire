#include "flag_data.h"
#include <Arduino.h> // Required for PROGMEM and basic types


// -----------------------------------------------------------------------------
// 5. Central Flag Lookup Table (Definition)
// -----------------------------------------------------------------------------
// This table holds all flag codes and pointers to their PROGMEM data.
const FlagEntry FLAG_LOOKUP[] = {
};

// Calculate the number of flags defined in the array
const int NUM_FLAGS = sizeof(FLAG_LOOKUP) / sizeof(FLAG_LOOKUP[0]);