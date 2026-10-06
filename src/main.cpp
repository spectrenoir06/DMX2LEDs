#include <Arduino.h>

#include <WS2812FX.h>

#include <FastLED.h>
#include <dmx_lib.h>

// From the config folder selected in platformio.ini (src/configs/<name>/)
#include "layout.h"
#include "anims.h"
#include "ota.h"

constexpr size_t NB_OUTPUTS = sizeof(OUTPUT_SIZES) / sizeof(OUTPUT_SIZES[0]);
constexpr uint16_t sum(const uint16_t* a, size_t n) { return n == 0 ? 0 : a[n - 1] + sum(a, n - 1); }
constexpr uint16_t LED_NB_PIXEL = sum(OUTPUT_SIZES, NB_OUTPUTS);

// Minimum time between two frames sent to the LEDs (100 fps max)
#define MIN_FRAME_MS 10

// WiFi update mode: fixture 1 at color R=42 G=43 B=44 with dim at 0, for
// OTA_HOLD_MS. Changing any of these values leaves update mode.
#define OTA_HOLD_MS 5000

const uint32_t max_num_seg = 20;
const uint32_t max_num_active_seg = 20;

static_assert(ZONE_COUNT <= 32, "zones are stored in a 32 bit mask");

CRGB leds[LED_NB_PIXEL];

// Computed from layout.h by init_layout()
uint16_t output_offset[NB_OUTPUTS];
uint16_t zone_start[ZONE_COUNT];
uint16_t zone_stop[ZONE_COUNT]; // inclusive
uint16_t max_dmx_channel = 0;   // last channel used, relative to the DIP address

const uint8_t DIP_ADDRESS_PINS[] = {
	DIP_PIN_0, DIP_PIN_1, DIP_PIN_2, DIP_PIN_3, DIP_PIN_4,
	DIP_PIN_5, DIP_PIN_6, DIP_PIN_7, DIP_PIN_8
};

// Runtime state of one fixture of FIXTURES (layout.h)
struct Fixture {
	const FixtureDef* def;
	WS2812FX* fx;        // its own animation engine, one pixel per LED of the fixture
	uint16_t led_start;  // first LED of the fixture in leds[]
	uint16_t led_count;
	Layout   layout = LAYOUT_LEDS;

	uint32_t color = 0xFF0000;
	int32_t  speed = 0;
	int32_t  strobe_speed = 0;
	bool     is_strobe = false;
	bool     blackout = false;
	unsigned long last_strobe_toggle = 0;
	int      anim = 255;
	uint8_t  bright = 255;
	bool     letter_mode = false;
	uint32_t letters_zones_on = 0;

	char     last_log[160] = "";
	unsigned long last_log_time = 0;
};

std::vector<Fixture> fixtures;
WS2812FX* current_fx = nullptr; // see custom_modes.h
bool leds_dirty = false;        // a fixture changed, leds[] must be rebuilt and sent
volatile bool leds_paused = false; // no frames sent while a firmware update writes the flash

// fixtures and the state above are shared between led_task and DMX_task
SemaphoreHandle_t fx_mutex;


// Custom show for every WS2812FX instance: the LED task sends everything at once
void mark_dirty(void) {
	leds_dirty = true;
}

void init_layout() {
	uint16_t next[NB_OUTPUTS];
	uint16_t offset = 0;
	for (uint8_t i = 0; i < NB_OUTPUTS; i++) {
		output_offset[i] = offset;
		next[i] = offset;
		offset += OUTPUT_SIZES[i];
	}

	for (uint8_t z = 0; z < ZONE_COUNT; z++) {
		uint8_t out = ZONES[z].output;
		zone_start[z] = next[out];
		next[out] += ZONES[z].count;
		zone_stop[z] = next[out] - 1;
		if (next[out] > output_offset[out] + OUTPUT_SIZES[out])
			Serial.printf("Layout error: zone %d overflows output %d\r\n", z, out);
	}

	for (const FixtureDef& def : FIXTURES) {
		Fixture f;
		f.def = &def;
		f.led_start = zone_start[def.zones.first];
		f.led_count = zone_stop[def.zones.last] - f.led_start + 1;
		// No pin: pixels are sent by FastLED, the instance only computes them
		f.fx = new WS2812FX(f.led_count, 0, NEO_RGB, max_num_seg, max_num_active_seg);
		f.fx->setPin(-1);
		// Set before anything calls show() (setBrightness, strip_off...), which
		// would otherwise make NeoPixel drive the RMT with the invalid pin.
		f.fx->setCustomShow(mark_dirty);
		fixtures.push_back(f);

		uint16_t last_channel = DMX_CHANNEL_ANIM;
		for (const Letter& l : def.letters)
			last_channel = max<uint16_t>(last_channel, l.dmx_channel);
		max_dmx_channel = max<uint16_t>(max_dmx_channel, def.dmx_offset + last_channel);
	}
}

// Copy a fixture's pixels into leds[], applying its dim and strobe
void render(Fixture& f) {
	const uint8_t* pixels = f.fx->getPixels();
	CRGB* out = &leds[f.led_start];

	if (f.layout == LAYOUT_LEDS) {
		memcpy(out, pixels, f.led_count * sizeof(CRGB));
	} else { // LAYOUT_ZONES: pixel n colors the whole zone n of the fixture
		fill_solid(out, f.led_count, CRGB::Black);
		for (uint8_t z = f.def->zones.first; z <= f.def->zones.last; z++) {
			uint8_t n = z - f.def->zones.first;
			fill_solid(&leds[zone_start[z]], ZONES[z].count, CRGB(pixels[n * 3], pixels[n * 3 + 1], pixels[n * 3 + 2]));
		}
	}

	uint8_t scale = f.blackout ? 0 : f.bright;
	if (scale != 255)
		nscale8(out, f.led_count, scale);
}

// Remove all segments, select the layout and turn the fixture off
void reset_fx(Fixture& f, Layout layout) {
	f.is_strobe = false;
	f.blackout = false;
	f.layout = layout;
	f.fx->resetSegments();
	f.fx->strip_off();
}

// Create the WS2812FX segment(s) for a zone range (relative to the fixture),
// starting at segment index n. Returns the next free segment index.
uint8_t add_segment(Fixture& f, uint8_t n, Range range, uint8_t mode, uint8_t flags) {
	uint8_t base = f.def->zones.first;
	uint16_t first = base + range.first;
	uint16_t last  = min<uint16_t>(base + range.last, f.def->zones.last);
	if (first > last)
		return n;

	if (flags & EACH_ZONE) {
		for (uint16_t z = first; z <= last; z++)
			n = add_segment(f, n, only(z - base), mode, flags & ~EACH_ZONE);
		return n;
	}

	if (n >= max_num_seg) {
		Serial.printf("Too many segments, max %d\r\n", max_num_seg);
		return n;
	}

	uint16_t start = (f.layout == LAYOUT_LEDS) ? zone_start[first] - f.led_start : first - base;
	uint16_t stop  = (f.layout == LAYOUT_LEDS) ? zone_stop[last]   - f.led_start : last  - base;
	f.fx->setSegment(n, start, stop, mode, f.color, 0, (bool)(flags & REVERSE));
	return n + 1;
}

void set_all_speed(Fixture& f) {
	for (uint8_t i = 0; i < f.fx->getNumSegments(); i++) {
		if (f.fx->getMode(i) == FX_MODE_STATIC) {
			f.fx->setSpeed(i, 1);
		} else {
			f.fx->setSpeed(i, f.speed);
		}
	}
}

void start_anim(Fixture& f) {
	if (f.anim < 0 || f.anim >= (int)ANIMS.size()) { // no animation: fixture stays off
		reset_fx(f, LAYOUT_LEDS);
		return;
	}

	const Anim& a = ANIMS[f.anim];
	reset_fx(f, a.layout);

	uint8_t n = 0;
	for (const Seg& s : a.segs)
		n = add_segment(f, n, s.range, s.mode, s.flags);

	if (a.strobe) {
		f.is_strobe = true;
		f.last_strobe_toggle = millis();
	}
	set_all_speed(f);
}

// Letter mode, see FixtureDef::letters. Returns false when no letter channel is used.
bool update_letters(Fixture& f, uint16_t dmx_base) {
	bool active = false;
	uint32_t zones_on = 0;
	for (const Letter& l : f.def->letters) {
		uint8_t value = DMXLibrary::Read(dmx_base + l.dmx_channel);
		if (value > 10)
			active = true;
		if (value > 127)
			zones_on |= l.zones;
	}
	if (!active)
		return false;

	if (!f.letter_mode)
		reset_fx(f, LAYOUT_LEDS);

	if (!f.letter_mode || zones_on != f.letters_zones_on) {
		f.letters_zones_on = zones_on;
		uint8_t base = f.def->zones.first;
		for (uint8_t z = base; z <= f.def->zones.last; z++) {
			uint8_t mode = (zones_on & ZONE_BIT(z)) ? FX_MODE_STATIC : MODE_OFF;
			add_segment(f, z - base, only(z - base), mode, 0);
		}
	}
	return true;
}

// Append printf-style text to a buffer, never overflowing it
static void appendf(char* buf, size_t size, const char* fmt, ...) {
	size_t len = strlen(buf);
	if (len + 1 >= size)
		return;
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf + len, size - len, fmt, args);
	va_end(args);
}

// Print a fixture's state when it changes, at most every 100 ms, e.g.
// [DMX 001 FOLLE] anim 05 wipe bulb + rainbow back | color #FF8000 | dim  80% | speed 128/255
// [DMX 001 FOLLE] letters F O -- -- E              | color #FF8000 | dim 100%
void log_state(Fixture& f, uint16_t dmx_base, uint8_t speed_channel) {
	char line[160] = "";

	appendf(line, sizeof(line), "[DMX %03d %s] ", dmx_base + 1, f.def->name);
	if (f.letter_mode) {
		char letters[48] = "letters";
		for (const Letter& l : f.def->letters)
			appendf(letters, sizeof(letters), " %s", (f.letters_zones_on & l.zones) ? l.name : "--");
		appendf(line, sizeof(line), "%-34s", letters);
	} else {
		appendf(line, sizeof(line), "anim %02d %-26s", f.anim, f.anim < (int)ANIMS.size() ? ANIMS[f.anim].name : "(none: off)");
	}
	appendf(line, sizeof(line), " | color #%06X | dim %3d%%", f.color, f.bright * 100 / 255);
	if (f.is_strobe)
		appendf(line, sizeof(line), " | strobe %3d/255 (%ld ms on/off)", speed_channel, f.strobe_speed / 10);
	else if (!f.letter_mode)
		appendf(line, sizeof(line), " | speed %3d/255 (fx %d)", speed_channel, (int)f.speed);

	unsigned long now = millis();
	if (strcmp(line, f.last_log) != 0 && now - f.last_log_time > 100) {
		Serial.println(line);
		strcpy(f.last_log, line);
		f.last_log_time = now;
	}
}

// DMX speed channel to WS2812FX speed: 0 = slowest (65535), 255 = fastest (10).
// Exponential, because modes divide the speed by up to the segment length:
// a linear curve leaves most of the fader below one frame for those modes.
uint16_t dmx_to_speed(uint8_t value) {
	return 10 * pow(6553.5, (255 - value) / 255.0);
}

void apply_dmx(Fixture& f, uint16_t dmx_base) {
	if (update_letters(f, dmx_base)) {
		f.letter_mode = true;
	} else {
		int new_anim = DMXLibrary::Read(dmx_base + DMX_CHANNEL_ANIM) / 17; // 0-15
		if (new_anim != f.anim || f.letter_mode) {
			f.anim = new_anim;
			start_anim(f);
		}
		f.letter_mode = false;
	}

	uint8_t r = DMXLibrary::Read(dmx_base + DMX_CHANNEL_COLOR_1_R);
	uint8_t g = DMXLibrary::Read(dmx_base + DMX_CHANNEL_COLOR_1_G);
	uint8_t b = DMXLibrary::Read(dmx_base + DMX_CHANNEL_COLOR_1_B);
	f.color = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
	f.fx->setAllColor(f.color);

	uint8_t bright = DMXLibrary::Read(dmx_base + DMX_CHANNEL_BRIGHT);
	if (bright != f.bright) {
		f.bright = bright;
		leds_dirty = true;
	}

	uint8_t speed_channel = DMXLibrary::Read(dmx_base + DMX_CHANNEL_SPEED);
	if (f.is_strobe) {
		f.strobe_speed = 2550 - speed_channel * 10;
	} else {
		int new_speed = dmx_to_speed(speed_channel);
		if (f.speed != new_speed) {
			f.speed = new_speed;
			set_all_speed(f);
		}
	}

	log_state(f, dmx_base, speed_channel);
}

uint16_t read_dip_address() {
	uint16_t address = 0;
	for (uint8_t i = 0; i < sizeof(DIP_ADDRESS_PINS); i++) {
		if (!digitalRead(DIP_ADDRESS_PINS[i]))
			address |= 1 << i;
	}
	return address;
}

// Same animation on every fixture, used in test mode and update mode
void play_on_all(uint8_t mode, uint16_t speed, uint8_t bright, uint32_t color = 0xFF0000) {
	for (Fixture& f : fixtures) {
		reset_fx(f, LAYOUT_LEDS);
		f.bright = bright;
		f.color = color;
		add_segment(f, 0, ALL, mode, 0);
		f.fx->setSpeed(0, speed);
		f.anim = 255; // restart the DMX animation when DMX comes back
		f.letter_mode = false;
	}
}

// LED feedback during a firmware update. Called from ota_loop() in DMX_task.
void on_ota_event(OtaEvent event) {
	xSemaphoreTake(fx_mutex, portMAX_DELAY);
	if (event == OTA_UPLOAD_START)
		play_on_all(FX_MODE_STATIC, 1000, 80, 0xFFC000); // yellow
	else
		play_on_all(FX_MODE_BREATH, 1000, 80, 0xFF0000); // red, update mode stays on
	xSemaphoreGive(fx_mutex);

	if (event == OTA_UPLOAD_START) {
		vTaskDelay(50 / portTICK_PERIOD_MS); // let the yellow frame go out
		leds_paused = true;
	} else {
		leds_paused = false;
	}
}

void enter_ota(uint16_t dip) {
	char name[32];
	snprintf(name, sizeof(name), "%s-%03d", DEVICE_NAME, dip);
	play_on_all(FX_MODE_BREATH, 1000, 80, 0x0040FF); // blue: waiting for a firmware
	ota_begin(name, on_ota_event);
}

void leave_ota() {
	ota_end();
	leds_paused = false;
	for (Fixture& f : fixtures)
		f.anim = 255; // restart the DMX animation
}

// Fixture 1 at the update mode color (R=42 G=43 B=44) with dim at 0
bool ota_requested(uint16_t dmx_adress) {
	uint16_t base = dmx_adress + fixtures[0].def->dmx_offset;
	return DMXLibrary::Read(base + DMX_CHANNEL_COLOR_1_R) == 42
		&& DMXLibrary::Read(base + DMX_CHANNEL_COLOR_1_G) == 43
		&& DMXLibrary::Read(base + DMX_CHANNEL_COLOR_1_B) == 44
		&& DMXLibrary::Read(base + DMX_CHANNEL_BRIGHT) == 0;
}

void DMX_task(void* parameter) {
	Serial.printf("Task DMX start\r\n");
	for (uint8_t pin : DIP_ADDRESS_PINS)
		pinMode(pin, INPUT_PULLUP);
	pinMode(DIP_PIN_9, INPUT_PULLUP); // unused

	pinMode(DMX_SERIAL_IO_PIN, OUTPUT);
	digitalWrite(DMX_SERIAL_IO_PIN, 0);

	delay(10);

	Serial.printf("DMX start init\r\n");
	DMXLibrary::Initialize(input);
	#ifdef INVERT_RX
		uart_set_line_inverse(2, UART_SIGNAL_RXD_INV);
	#endif
	Serial.println("DMX initialized...");
	Serial.printf("Adress DMX: %d, %d fixture(s), %d channels\r\n", read_dip_address(), (int)fixtures.size(), max_dmx_channel);

	uint8_t ctn = 0;
	uint8_t test_mode = 0;
	enum { STATUS_NONE, STATUS_OK, STATUS_NO_SIGNAL, STATUS_BAD_ADDRESS } last_status = STATUS_NONE;
	bool ota_holding = false;
	unsigned long ota_hold_since = 0;
	uint32_t dropped_logged = 0;
	unsigned long dropped_log_time = 0;

	for (;;) {
		ota_loop();
		uint16_t dip = read_dip_address();

		// glitches on the DMX line: incomplete frames ignored by the DMX library
		uint32_t dropped = DMXLibrary::DroppedFrames();
		if (dropped != dropped_logged && millis() - dropped_log_time > 1000) {
			Serial.printf("[DMX] %u incomplete frame(s) ignored (total %u)\r\n", dropped - dropped_logged, dropped);
			dropped_logged = dropped;
			dropped_log_time = millis();
		}

		xSemaphoreTake(fx_mutex, portMAX_DELAY);
		if (dip == 0 && !ota_active()) { // test mode
			if (test_mode == 0) {
				Serial.printf("[TEST] DIP address is 0: rainbow test pattern\r\n");
				last_status = STATUS_NONE;
				play_on_all(FX_MODE_RAINBOW_CYCLE, 10, 50);
				test_mode = 1;
			}
		} else {
			test_mode = 0;
			uint16_t dmx_adress = dip - 1;
			auto status = STATUS_OK;
			if (dmx_adress + max_dmx_channel > TOTAL_CHANNELS)
				status = STATUS_BAD_ADDRESS;
			else if (!DMXLibrary::IsHealthy())
				status = STATUS_NO_SIGNAL;

			if (status != last_status) {
				if (status == STATUS_OK)
					Serial.printf("[DMX %03d] signal OK\r\n", dip);
				else if (status == STATUS_NO_SIGNAL)
					Serial.printf("[DMX %03d] no DMX signal\r\n", dip);
				else
					Serial.printf("[DMX %03d] address too high, max is %d\r\n", dip, TOTAL_CHANNELS - max_dmx_channel + 1);
				last_status = status;
			}

			if (status == STATUS_OK) {
				digitalWrite(LED_STATUS_PIN, HIGH);
				ctn = 0;

				if (ota_active()) {
					if (!ota_requested(dmx_adress))
						leave_ota();
				} else if (ota_requested(dmx_adress)) {
					if (!ota_holding) {
						ota_holding = true;
						ota_hold_since = millis();
						Serial.printf("[OTA] update mode requested, hold for %d s\r\n", OTA_HOLD_MS / 1000);
					} else if (millis() - ota_hold_since >= OTA_HOLD_MS) {
						ota_holding = false;
						enter_ota(dip);
					}
				} else {
					ota_holding = false;
				}

				if (!ota_active()) {
					for (Fixture& f : fixtures)
						apply_dmx(f, dmx_adress + f.def->dmx_offset);
				}
			} else if (ctn++ > 10) {
				digitalWrite(LED_STATUS_PIN, LOW);
			}
		}
		xSemaphoreGive(fx_mutex);

		vTaskDelay(25 / portTICK_PERIOD_MS);
	}
}

void led_task(void* parameter) {
	Serial.printf("Task LED start\r\n");

	add_outputs(leds, output_offset);

	xSemaphoreTake(fx_mutex, portMAX_DELAY);
	for (Fixture& f : fixtures) {
		register_custom_modes(*f.fx);
		f.fx->init();
		f.fx->setBrightness(255);
		f.fx->start();
		// WS2812FX starts as static red: stay dark until DMX is received, and
		// send a black frame to clear anything the strip latched during the reset
		reset_fx(f, LAYOUT_LEDS);
	}
	xSemaphoreGive(fx_mutex);

	pinMode(LED_STATUS_PIN, OUTPUT);
	digitalWrite(LED_STATUS_PIN, LOW);

	unsigned long last_frame = 0;

	for (;;) {
		xSemaphoreTake(fx_mutex, portMAX_DELAY);
		unsigned long now = millis();
		for (Fixture& f : fixtures) {
			current_fx = f.fx;
			f.fx->service();
			if (f.is_strobe && now - f.last_strobe_toggle > (unsigned long)(f.strobe_speed / 10)) { // 0-255 ms half period
				f.blackout = !f.blackout;
				f.last_strobe_toggle = now;
				leds_dirty = true;
			}
		}

		bool send = leds_dirty && !leds_paused && now - last_frame >= MIN_FRAME_MS;
		if (send) {
			for (Fixture& f : fixtures)
				render(f);
			leds_dirty = false;
			last_frame = now;
		}
		xSemaphoreGive(fx_mutex);

		// leds[] is only written by this task: send it without blocking the DMX task
		if (send)
			FastLED.show();
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}

void setup() {
	Serial.begin(115200);
	Serial.printf("Start\r\n");

	init_layout();
	fx_mutex = xSemaphoreCreateMutex();

	xTaskCreatePinnedToCore(
		led_task,    // Function that should be called
		"led_task",   // Name of the task (for debugging)
		10000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		1,               // Task priority
		NULL,             // Task handle
		0          // Core you want to run the task on (0 or 1)
	);

	xTaskCreatePinnedToCore(
		DMX_task,    // Function that should be called
		"DMX_task",   // Name of the task (for debugging)
		10000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		1,               // Task priority
		NULL,             // Task handle
		1          // Core you want to run the task on (0 or 1)
	);
}

void loop() {
	vTaskDelete(NULL); // everything runs in led_task and DMX_task
}
