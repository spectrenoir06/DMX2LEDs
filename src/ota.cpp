// Firmware update over WiFi: the board creates its own network (access point)
// and accepts a new firmware.bin from a browser (http://4.3.2.1) or from
// PlatformIO (ArduinoOTA: pio run -e <env> -t upload --upload-port 4.3.2.1).
// The new firmware is only used once it is fully written and checked, so an
// interrupted upload keeps the old one.
// Only devices that joined the network (WPA2, OTA_PASSWORD) can upload, so
// ArduinoOTA has no password of its own.
#include "ota.h"
#include "espnow_input.h"

#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include <ArduinoOTA.h>

#ifndef OTA_PASSWORD
#error "OTA_PASSWORD must be defined (platformio.ini, [ota] section)"
#endif
static_assert(sizeof(OTA_PASSWORD) - 1 >= 8, "OTA_PASSWORD needs at least 8 characters (WiFi WPA2)");

static const IPAddress AP_IP(4, 3, 2, 1);

static WebServer server(80);
static bool active = false;
static void (*event_cb)(OtaEvent) = nullptr;

static const char UPLOAD_PAGE[] =
	"<!doctype html><html><head><meta name=viewport content='width=device-width'>"
	"<title>Firmware update</title></head><body style='font-family:sans-serif'>"
	"<h2>Firmware update</h2>"
	"<form method=POST action=/update enctype=multipart/form-data>"
	"<input type=file name=firmware accept=.bin> <input type=submit value=Upload>"
	"</form><p>Select .pio/build/&lt;env&gt;/firmware.bin. The board reboots when done.</p>"
	"</body></html>";

static void on_upload_done() {
	server.sendHeader("Connection", "close");
	if (Update.hasError()) {
		server.send(500, "text/plain", "Update failed, see the serial log");
		return;
	}
	server.send(200, "text/plain", "Update OK, rebooting");
	Serial.printf("[OTA] update OK, rebooting\r\n");
	delay(500);
	ESP.restart();
}

static void on_upload_chunk() {
	HTTPUpload& upload = server.upload();
	if (upload.status == UPLOAD_FILE_START) {
		Serial.printf("[OTA] receiving %s\r\n", upload.filename.c_str());
		if (event_cb)
			event_cb(OTA_UPLOAD_START);
		if (!Update.begin(UPDATE_SIZE_UNKNOWN))
			Update.printError(Serial);
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
			Update.printError(Serial);
	} else if (upload.status == UPLOAD_FILE_END) {
		if (Update.end(true)) {
			Serial.printf("[OTA] %u bytes written\r\n", upload.totalSize);
		} else {
			Update.printError(Serial);
			if (event_cb)
				event_cb(OTA_UPLOAD_ERROR);
		}
	} else if (upload.status == UPLOAD_FILE_ABORTED) {
		Update.abort();
		Serial.printf("[OTA] upload aborted\r\n");
		if (event_cb)
			event_cb(OTA_UPLOAD_ERROR);
	}
}

void ota_begin(const char* network_name, void (*on_event)(OtaEvent)) {
	if (active)
		return;
	event_cb = on_event;

	// With ESP-NOW on, keep the station interface, which receives it. Both
	// interfaces share one radio: the access point uses its default channel
	// 1, the ESP-NOW channel (espnow_input.h).
	WiFi.mode(espnow_enabled() ? WIFI_AP_STA : WIFI_AP);
	WiFi.softAP(network_name, OTA_PASSWORD);
	WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0)); // short address, like WLED

	server.on("/", HTTP_GET, []() { server.send(200, "text/html", UPLOAD_PAGE); });
	server.on("/update", HTTP_POST, on_upload_done, on_upload_chunk);
	server.begin();

	ArduinoOTA.setHostname(network_name);
	ArduinoOTA.onStart([]() {
		Serial.printf("[OTA] receiving firmware from PlatformIO\r\n");
		if (event_cb)
			event_cb(OTA_UPLOAD_START);
	});
	ArduinoOTA.onEnd([]() { Serial.printf("[OTA] update OK, rebooting\r\n"); });
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("[OTA] PlatformIO upload failed (error %d)\r\n", error);
		if (event_cb)
			event_cb(OTA_UPLOAD_ERROR);
	});
	ArduinoOTA.begin();

	active = true;
	Serial.printf("[OTA] WiFi on: network \"%s\", open http://%s\r\n",
		network_name, WiFi.softAPIP().toString().c_str());
}

void ota_end() {
	if (!active)
		return;
	ArduinoOTA.end();
	server.stop();
	WiFi.softAPdisconnect(true);
	if (!espnow_enabled())
		WiFi.mode(WIFI_OFF); // with ESP-NOW on, WiFi stays on for the receiver
	active = false;
	Serial.printf("[OTA] WiFi off\r\n");
}

// Blocks during an upload (the whole firmware is received in one call)
void ota_loop() {
	if (!active)
		return;
	server.handleClient();
	ArduinoOTA.handle();
}

bool ota_active() {
	return active;
}
