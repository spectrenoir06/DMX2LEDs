// Animation list, selected by the DMX anim channel (value / 17).
// Edit this file to add, remove or change animations.
#pragma once

#include "fx_types.h"
#include "custom_modes.h"
#include "layout.h"

// Ranges are zones of the fixture: ALL, ALL_BACK, ALL_BULB, only(O_BACK)...
// Flags: REVERSE, EACH_ZONE. Layouts are explained in fx_types.h.
// Speed comes from the DMX speed channel. Color is the main DMX color.
// At most 20 segments per animation (EACH_ZONE counts one per zone).
const std::vector<Anim> ANIMS = {
	/*  0 */ {"static", LAYOUT_LEDS, {
		{ALL, FX_MODE_STATIC},
	}},
	/*  1 */ {"strobe", LAYOUT_LEDS, {
		{ALL, FX_MODE_STATIC},
	}, true},
	/*  2 */ {"wipe left + left", LAYOUT_ZONES, {
		{ALL_BACK, FX_MODE_COLOR_WIPE_INV},
		{ALL_BULB, FX_MODE_COLOR_WIPE_INV},
	}},
	/*  3 */ {"wipe left + right", LAYOUT_ZONES, {
		{ALL_BACK, FX_MODE_COLOR_WIPE_INV},
		{ALL_BULB, FX_MODE_COLOR_WIPE_INV, REVERSE},
	}},
	/*  4 */ {"wipe right + left", LAYOUT_ZONES, {
		{ALL_BACK, FX_MODE_COLOR_WIPE_INV, REVERSE},
		{ALL_BULB, FX_MODE_COLOR_WIPE_INV},
	}},
	/*  5 */ {"wipe bulb + rainbow back", LAYOUT_LEDS, {
		{ALL_BULB, FX_MODE_COLOR_WIPE_INV, EACH_ZONE},
		{ALL_BACK, FX_MODE_RAINBOW_CYCLE,  EACH_ZONE},
	}},
	/*  6 */ {"rainbow bulb + wipe back", LAYOUT_LEDS, {
		{ALL_BULB, FX_MODE_RAINBOW_CYCLE,  EACH_ZONE},
		{ALL_BACK, FX_MODE_COLOR_WIPE_INV, EACH_ZONE},
	}},
	/*  7 */ {"wipe per letter", LAYOUT_LEDS, {
		{ALL, FX_MODE_COLOR_WIPE_INV, EACH_ZONE},
	}},
	/*  8 */ {"theater chase", LAYOUT_LEDS, {
		{ALL, FX_MODE_THEATER_CHASE, EACH_ZONE},
	}},
	/*  9 */ {"multi strobe", LAYOUT_LEDS, {
		{ALL, FX_MODE_MULTI_STROBE},
	}},
	/* 10 */ {"twinklefox", LAYOUT_LEDS, {
		{ALL, FX_MODE_TWINKLEFOX},
	}},
	/* 11 */ {"fire flicker", LAYOUT_LEDS, {
		{ALL, FX_MODE_FIRE_FLICKER},
	}},
	/* 12 */ {"hyper sparkle", LAYOUT_LEDS, {
		{ALL, FX_MODE_HYPER_SPARKLE},
	}},
	/* 13 */ {"running lights", LAYOUT_LEDS, {
		{ALL, FX_MODE_RUNNING_LIGHTS},
	}},
	/* 14 */ {"rainbow O", LAYOUT_LEDS, {
		{only(O_BACK), FX_MODE_RAINBOW_CYCLE},
		{only(O_BULB), FX_MODE_RAINBOW_CYCLE},
	}},
};
