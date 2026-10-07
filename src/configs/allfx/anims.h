// Every WS2812FX effect, for testing them all: the DMX anim channel value
// is the effect id (FX_MODE_STATIC = 0 ... FX_MODE_TWINKLEFOX = 55), then
// the custom modes off (56) and snake (57). Higher values: fixture off.
// Each effect covers the whole fixture, with the main DMX color.
#pragma once

#include "fx_types.h"
#include "custom_modes.h"
#include "layout.h"

#define ANIM_DMX_STEP 1 // anim = DMX value, not value / 17

inline std::vector<Anim> all_effects() {
	std::vector<Anim> anims;
	// Names from WS2812FX. _names is a static copy in each file: the custom
	// mode names set in WS2812FX.cpp don't show here, so they are added by hand.
	for (uint8_t m = 0; m < FX_MODE_CUSTOM_0; m++)
		anims.push_back({(const char*)_names[m], LAYOUT_LEDS, {{ALL, m}}});
	anims.push_back({"off", LAYOUT_LEDS, {{ALL, MODE_OFF}}});
	anims.push_back({"snake", LAYOUT_LEDS, {{ALL, MODE_SNAKE}}});
	return anims;
}

const std::vector<Anim> ANIMS = all_effects();
