// Animation list, selected by the DMX anim channel (value / 17).
// Edit this file to add, remove or change animations.
#pragma once

#include "fx_types.h"
#include "custom_modes.h"
#include "layout.h"

// Each fixture is a single strip: use ALL as range.
// Speed comes from the DMX speed channel. Color is the main DMX color.
const std::vector<Anim> ANIMS = {
	/*  0 */ {"static",         LAYOUT_LEDS, {{ALL, FX_MODE_STATIC}}},
	/*  1 */ {"strobe",         LAYOUT_LEDS, {{ALL, FX_MODE_STATIC}}, true},
	/*  2 */ {"color wipe",     LAYOUT_LEDS, {{ALL, FX_MODE_COLOR_WIPE}}},
	/*  3 */ {"theater chase",  LAYOUT_LEDS, {{ALL, FX_MODE_THEATER_CHASE}}},
	/*  4 */ {"fireworks",      LAYOUT_LEDS, {{ALL, FX_MODE_FIREWORKS}}},
	/*  5 */ {"running lights", LAYOUT_LEDS, {{ALL, FX_MODE_RUNNING_LIGHTS}}},
	/*  6 */ {"twinklefox",     LAYOUT_LEDS, {{ALL, FX_MODE_TWINKLEFOX}}},
	/*  7 */ {"fire flicker",   LAYOUT_LEDS, {{ALL, FX_MODE_FIRE_FLICKER}}},
	/*  8 */ {"comet",          LAYOUT_LEDS, {{ALL, FX_MODE_COMET}}},
	/*  9 */ {"snake",          LAYOUT_LEDS, {{ALL, MODE_SNAKE}}},
	/* 10 */ {"rainbow",        LAYOUT_LEDS, {{ALL, FX_MODE_RAINBOW}}},
	/* 11 */ {"rainbow cycle",  LAYOUT_LEDS, {{ALL, FX_MODE_RAINBOW_CYCLE}}},
	/* 12 */ {"multi strobe",   LAYOUT_LEDS, {{ALL, FX_MODE_MULTI_STROBE}}},
	/* 13 */ {"hyper sparkle",  LAYOUT_LEDS, {{ALL, FX_MODE_HYPER_SPARKLE}}},
	/* 14 */ {"larson scanner", LAYOUT_LEDS, {{ALL, FX_MODE_LARSON_SCANNER}}},
	/* 15 */ {"breath",         LAYOUT_LEDS, {{ALL, FX_MODE_BREATH}}},
};
