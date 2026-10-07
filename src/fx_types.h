// Types used by the configs in src/configs/<name>/ (layout.h and anims.h).
#pragma once

#include <Arduino.h>
#include <vector>

// ---- DMX block ---------------------------------------------------------
#define TOTAL_CHANNELS 512

// Channels of each fixture, relative to its block (1 = first channel).
// Letter channels are defined per fixture in layout.h.
#define DMX_CHANNEL_COLOR_1_R    ( 1)
#define DMX_CHANNEL_COLOR_1_G    ( 2)
#define DMX_CHANNEL_COLOR_1_B    ( 3)
#define DMX_CHANNEL_BRIGHT       ( 4)
#define DMX_CHANNEL_SPEED        ( 5)
#define DMX_CHANNEL_ANIM         ( 6)

// ---- Layout ------------------------------------------------------------

// Named group of consecutive LEDs on one output
struct ZoneDef {
	uint8_t     output;
	uint16_t    count;
	const char* name; // optional, shown by the LED preview (scripts/led_view.py)
};

#define ZONE_BIT(z) (1UL << (z))

// Range of consecutive zones. In animations it is relative to the
// fixture's first zone and clipped to its last zone.
struct Range {
	uint8_t first;
	uint8_t last;
};

constexpr Range ALL = {0, 255}; // every zone of the fixture
constexpr Range only(uint8_t z) { return {z, z}; }

// Letter mode: when any letter channel is above 10, each letter's zones
// are lit with the main color while its channel is above 127.
struct Letter {
	const char* name;        // for the serial log
	uint8_t     dmx_channel; // relative to the fixture's DMX block (1 = first channel)
	uint32_t    zones;       // ZONE_BIT(...) | ZONE_BIT(...)
};

// A fixture plays one animation on a range of zones, controlled by its own
// block of DMX channels (color, dim, speed, anim, optional letters).
struct FixtureDef {
	const char*         name;       // for the serial log
	Range               zones;      // absolute zone indexes
	uint16_t            dmx_offset; // where its DMX block starts, relative to the DIP address
	std::vector<Letter> letters;    // optional
};

// ---- Animations --------------------------------------------------------

// LAYOUT_LEDS:  one WS2812FX pixel per LED. A segment covers every LED
//               of its zone range.
// LAYOUT_ZONES: one WS2812FX pixel per zone; each zone shows a single
//               color. A segment over 5 zones is 5 pixels long, so a
//               wipe moves zone by zone.
enum Layout : uint8_t { LAYOUT_LEDS, LAYOUT_ZONES };

// Segment flags
#define REVERSE   (1 << 0)
#define EACH_ZONE (1 << 1) // one segment per zone in the range instead of one for the whole range

struct Seg {
	Range   range;
	uint8_t mode;
	uint8_t flags; // optional
};

struct Anim {
	const char*      name;
	Layout           layout;
	std::vector<Seg> segs;
	bool             strobe; // optional: blink the whole fixture, rate from the speed channel
};
