// ESP-NOW receiver for two kinds of packets, told apart by their size:
//  - DMX from dmx2espnow: channels 1-250, one byte each (ESP-NOW payloads are
//    250 bytes max, so higher channels read as 0)
//  - WizMote remote: 13 bytes, program 0x91 (ON button) or 0x81, a 32 bit
//    sequence number (LSB first, the same packet can arrive twice), button
// Only paired senders are accepted (MAC address list, saved in flash): this
// keeps other remotes and systems nearby out, but a MAC can be faked, so it
// is no protection against someone doing it on purpose.
// Pairing mode (DIP address 0 + switch 10, see main.cpp): every sender heard
// is paired, the first one replacing the previous list.
// Only started when DIP switch 10 is ON at power-up: WiFi stays off otherwise.
#include "espnow_input.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Preferences.h>

#define DMX_TIMEOUT_MS   500
#define WIZMOTE_LEN      13
#define MIN_DMX_LEN      16 // shorter packets are not DMX

static uint8_t dmx_data[513];          // [channel], 1-512
static volatile unsigned long last_dmx = 0;
static bool dmx_received = false;
static portMUX_TYPE dmx_lock = portMUX_INITIALIZER_UNLOCKED;
static QueueHandle_t buttons;
static bool enabled = false;

#define MAX_PAIRED 8
static uint8_t paired[MAX_PAIRED][6];    // guarded by dmx_lock (read in the WiFi task)
static uint8_t nb_paired = 0;
static volatile bool pairing = false;
static QueueHandle_t heard;              // pairing mode: MAC addresses of the senders heard
static uint8_t last_refused[6];          // last unpaired sender, for the log
static volatile uint32_t refused = 0;
static Preferences prefs;

static bool is_paired(const uint8_t* mac) {
	for (uint8_t i = 0; i < nb_paired; i++)
		if (memcmp(paired[i], mac, 6) == 0)
			return true;
	return false;
}

// Runs in the WiFi task: copy and return
static void on_receive(const uint8_t* mac, const uint8_t* data, int len) {
	if (pairing) { // packets are only used to pair, not applied
		uint8_t m[6];
		memcpy(m, mac, 6);
		xQueueSend(heard, m, 0);
		return;
	}
	portENTER_CRITICAL(&dmx_lock);
	bool ok = is_paired(mac);
	if (!ok) {
		memcpy(last_refused, mac, 6);
		refused++;
	}
	portEXIT_CRITICAL(&dmx_lock);
	if (!ok)
		return;

	if (len == WIZMOTE_LEN && (data[0] == 0x91 || data[0] == 0x81)) {
		static uint32_t last_seq = UINT32_MAX;
		uint32_t seq = data[1] | data[2] << 8 | data[3] << 16 | (uint32_t)data[4] << 24;
		if (seq == last_seq)
			return;
		last_seq = seq;
		uint8_t button = data[6];
		xQueueSend(buttons, &button, 0);
	} else if (len >= MIN_DMX_LEN && len <= 512) {
		portENTER_CRITICAL(&dmx_lock);
		memcpy(dmx_data + 1, data, len);
		memset(dmx_data + 1 + len, 0, 512 - len);
		last_dmx = millis();
		dmx_received = true;
		portEXIT_CRITICAL(&dmx_lock);
	}
}

void espnow_resume() {
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
}

void espnow_begin() {
	buttons = xQueueCreate(8, sizeof(uint8_t));
	heard = xQueueCreate(8, 6);
	prefs.begin("espnow", false);
	if (prefs.isKey("paired")) {
		size_t len = prefs.getBytes("paired", paired, sizeof(paired));
		nb_paired = len / 6;
	}
	espnow_resume();
	if (esp_now_init() != ESP_OK) {
		Serial.printf("[ESP-NOW] init failed\r\n");
		return;
	}
	esp_now_register_recv_cb(on_receive);
	enabled = true;
	Serial.printf("[ESP-NOW] listening on WiFi channel %d, %d paired sender(s)%s\r\n", ESPNOW_CHANNEL,
		nb_paired, nb_paired ? "" : ": pair with DIP address 0 + switch 10");
}

void espnow_set_pairing(bool on) {
	if (!enabled || on == pairing)
		return;
	pairing = on;
	Serial.printf(on ? "[ESP-NOW] pairing mode: press a button on the remote, start dmx2espnow\r\n"
	                 : "[ESP-NOW] pairing mode off, %d paired sender(s)\r\n", nb_paired);
}

void espnow_poll() {
	if (!enabled)
		return;
	static bool session_started = false; // the first sender of a session replaces the list
	if (!pairing)
		session_started = false;

	uint8_t mac[6];
	while (xQueueReceive(heard, mac, 0) == pdTRUE) {
		portENTER_CRITICAL(&dmx_lock);
		bool known = session_started && is_paired(mac);
		if (!known) {
			if (!session_started)
				nb_paired = 0;
			if (nb_paired < MAX_PAIRED)
				memcpy(paired[nb_paired++], mac, 6);
		}
		portEXIT_CRITICAL(&dmx_lock);
		if (!known) {
			session_started = true;
			prefs.putBytes("paired", paired, nb_paired * 6);
			Serial.printf("[ESP-NOW] paired %02X:%02X:%02X:%02X:%02X:%02X (%d sender(s))\r\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], nb_paired);
		}
	}

	// Refused packets, at most every 5 s
	static uint32_t refused_logged = 0;
	static unsigned long refused_log_time = 0;
	if (refused != refused_logged && millis() - refused_log_time > 5000) {
		portENTER_CRITICAL(&dmx_lock);
		uint8_t m[6];
		memcpy(m, last_refused, 6);
		uint32_t n = refused;
		portEXIT_CRITICAL(&dmx_lock);
		Serial.printf("[ESP-NOW] %u packet(s) ignored, not paired: last from %02X:%02X:%02X:%02X:%02X:%02X\r\n",
			n - refused_logged, m[0], m[1], m[2], m[3], m[4], m[5]);
		refused_logged = n;
		refused_log_time = millis();
	}
}

bool espnow_enabled() {
	return enabled;
}

bool espnow_dmx_healthy() {
	return enabled && dmx_received && millis() - last_dmx < DMX_TIMEOUT_MS;
}

uint8_t espnow_dmx_read(uint16_t channel) {
	if (channel < 1 || channel > 512)
		return 0;
	portENTER_CRITICAL(&dmx_lock);
	uint8_t value = dmx_data[channel];
	portEXIT_CRITICAL(&dmx_lock);
	return value;
}

int espnow_take_button() {
	uint8_t button;
	return enabled && xQueueReceive(buttons, &button, 0) == pdTRUE ? button : -1;
}
