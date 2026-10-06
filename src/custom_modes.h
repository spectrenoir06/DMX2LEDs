// Custom WS2812FX modes, usable in animations like any FX_MODE_*.
// To add one, write the function, give it the next FX_MODE_CUSTOM_n id
// and register it in register_custom_modes().
#pragma once

#include <WS2812FX.h>

#define MODE_OFF   FX_MODE_CUSTOM_0
#define MODE_SNAKE FX_MODE_CUSTOM_1

// The instance being serviced: custom modes get no argument, so the
// engine sets this before each service() call.
extern WS2812FX* current_fx;

static uint16_t mode_off(void) {
	WS2812FX::Segment* seg = current_fx->getSegment();
	for (uint16_t i = seg->start; i <= seg->stop; i++)
		current_fx->setPixelColor(i, 0);
	return seg->speed / 25; // delay until the next step (ms)
}

static uint16_t mode_snake(void) { // chase of 3 on / 3 off
	WS2812FX::Segment* seg = current_fx->getSegment();
	uint32_t* counter = &current_fx->getSegmentRuntime()->counter_mode_step;

	for (uint16_t i = seg->stop; i > seg->start; i--)
		current_fx->setPixelColor(i, current_fx->getPixelColor(i - 1));

	*counter = (*counter + 1) % 6;
	current_fx->setPixelColor(seg->start, *counter < 3 ? seg->colors[0] : 0);

	return seg->speed / 25;
}

inline void register_custom_modes(WS2812FX& fx) {
	fx.setCustomMode(MODE_OFF   - FX_MODE_CUSTOM_0, F("off"),   mode_off);
	fx.setCustomMode(MODE_SNAKE - FX_MODE_CUSTOM_0, F("snake"), mode_snake);
}
