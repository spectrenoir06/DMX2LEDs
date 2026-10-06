// Generic LED strip controller.
//   default:        one output (LED_PIN_1) driven as a single fixture
//   MULTI_OUTPUT:   8 outputs, each one an independent fixture with its own
//                   DMX block (output 1 at the address, output 2 at +6...)
// LED_COUNT (LEDs per output) and LED_COLOR_ORDER come from platformio.ini.
#pragma once

#include <FastLED.h>
#include "fx_types.h"

#ifndef LED_COLOR_ORDER
#define LED_COLOR_ORDER BGR
#endif

#define DMX_BLOCK_SIZE 6 // R G B, dim, speed, anim

#ifndef MULTI_OUTPUT

constexpr uint16_t OUTPUT_SIZES[] = { LED_COUNT };

inline void add_outputs(CRGB* leds, const uint16_t* offset) {
	FastLED.addLeds<WS2812B, LED_PIN_1, LED_COLOR_ORDER>(leds, offset[0], LED_COUNT);
}

enum Zone : uint8_t { STRIP, ZONE_COUNT };

const ZoneDef ZONES[ZONE_COUNT] = {
	{0, LED_COUNT},
};

const std::vector<FixtureDef> FIXTURES = {
	{"strip", only(STRIP), 0},
};

#else

constexpr uint16_t OUTPUT_SIZES[] = {
	LED_COUNT, LED_COUNT, LED_COUNT, LED_COUNT,
	LED_COUNT, LED_COUNT, LED_COUNT, LED_COUNT,
};

inline void add_outputs(CRGB* leds, const uint16_t* offset) {
	FastLED.addLeds<WS2812B, LED_PIN_1, LED_COLOR_ORDER>(leds, offset[0], LED_COUNT);
	FastLED.addLeds<WS2812B, LED_PIN_2, LED_COLOR_ORDER>(leds, offset[1], LED_COUNT);
	FastLED.addLeds<WS2812B, LED_PIN_3, LED_COLOR_ORDER>(leds, offset[2], LED_COUNT);
	FastLED.addLeds<WS2812B, LED_PIN_4, LED_COLOR_ORDER>(leds, offset[3], LED_COUNT);
	FastLED.addLeds<WS2812B, LED_PIN_5, LED_COLOR_ORDER>(leds, offset[4], LED_COUNT);
	FastLED.addLeds<WS2812B, LED_PIN_6, LED_COLOR_ORDER>(leds, offset[5], LED_COUNT);
	FastLED.addLeds<WS2812B, LED_PIN_7, LED_COLOR_ORDER>(leds, offset[6], LED_COUNT);
	FastLED.addLeds<WS2812B, LED_PIN_8, LED_COLOR_ORDER>(leds, offset[7], LED_COUNT);
}

enum Zone : uint8_t { OUT_1, OUT_2, OUT_3, OUT_4, OUT_5, OUT_6, OUT_7, OUT_8, ZONE_COUNT };

const ZoneDef ZONES[ZONE_COUNT] = {
	{0, LED_COUNT}, {1, LED_COUNT}, {2, LED_COUNT}, {3, LED_COUNT},
	{4, LED_COUNT}, {5, LED_COUNT}, {6, LED_COUNT}, {7, LED_COUNT},
};

const std::vector<FixtureDef> FIXTURES = {
	{"out 1", only(OUT_1), 0 * DMX_BLOCK_SIZE},
	{"out 2", only(OUT_2), 1 * DMX_BLOCK_SIZE},
	{"out 3", only(OUT_3), 2 * DMX_BLOCK_SIZE},
	{"out 4", only(OUT_4), 3 * DMX_BLOCK_SIZE},
	{"out 5", only(OUT_5), 4 * DMX_BLOCK_SIZE},
	{"out 6", only(OUT_6), 5 * DMX_BLOCK_SIZE},
	{"out 7", only(OUT_7), 6 * DMX_BLOCK_SIZE},
	{"out 8", only(OUT_8), 7 * DMX_BLOCK_SIZE},
};

#endif
