// FOLLE sign: LED outputs, zones, and the fixture with its letter channels.
// Edit this file to adapt the firmware to a different mapping.
#pragma once

#include <FastLED.h>
#include "fx_types.h"

#define DEVICE_NAME "FOLLE" // WiFi network name in update mode: FOLLE-<address>

// ---- Outputs -----------------------------------------------------------
// Number of LEDs on each data pin. All outputs share one pixel buffer,
// stored back to back in this order.
constexpr uint16_t OUTPUT_SIZES[] = {
	399,     // output 0: background
	55 + 11, // output 1: lightbulb
};

// FastLED needs the pin and color order at compile time, so each output
// is declared here. `offset` is where the output starts in the buffer.
inline void add_outputs(CRGB* leds, const uint16_t* offset) {
	FastLED.addLeds<WS2812B, LED_PIN_1, GRB>(leds, offset[0], OUTPUT_SIZES[0]);
	FastLED.addLeds<WS2812B, LED_PIN_2, RGB>(leds, offset[1], OUTPUT_SIZES[1]);
}

// ---- Zones -------------------------------------------------------------
// Named groups of consecutive LEDs. Animations and letters address zones,
// never raw LED indexes.
// Zones are laid out in the order listed: each one starts where the
// previous zone on the same output ended. Add a zone for unused LEDs if
// there is a gap.
enum Zone : uint8_t {
	F_BACK, O_BACK, L1_BACK, L2_BACK, E_BACK,
	F_BULB, O_BULB, L1_BULB, L2_BULB, E_BULB,
	ZONE_COUNT
};

const ZoneDef ZONES[ZONE_COUNT] = {
	// output, nb LEDs
	{0, 85}, // F_BACK
	{0, 70}, // O_BACK
	{0, 75}, // L1_BACK
	{0, 74}, // L2_BACK
	{0, 95}, // E_BACK

	{1, 13}, // F_BULB
	{1, 16}, // O_BULB
	{1, 11}, // L1_BULB
	{1, 11}, // L2_BULB
	{1, 15}, // E_BULB
};

// Ranges of consecutive zones, used by animations (see anims.h)
constexpr Range ALL_BACK = {F_BACK, E_BACK};
constexpr Range ALL_BULB = {F_BULB, E_BULB};

// ---- Fixtures ----------------------------------------------------------
// One fixture for the whole sign. DMX block, relative to the DIP address:
// 1-6 color R G B, dim, speed, anim, then 7-11 the letters.
const std::vector<FixtureDef> FIXTURES = {
	{"FOLLE", {0, ZONE_COUNT - 1}, 0, {
		{"F",   7, ZONE_BIT(F_BACK)  | ZONE_BIT(F_BULB)  },
		{"O",   8, ZONE_BIT(O_BACK)  | ZONE_BIT(O_BULB)  },
		{"L1",  9, ZONE_BIT(L1_BACK) | ZONE_BIT(L1_BULB) },
		{"L2", 10, ZONE_BIT(L2_BACK) | ZONE_BIT(L2_BULB) },
		{"E",  11, ZONE_BIT(E_BACK)  | ZONE_BIT(E_BULB)  },
	}},
};
