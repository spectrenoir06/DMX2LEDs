// Firmware update over WiFi (see ota.cpp). Started from DMX by main.cpp.
#pragma once

enum OtaEvent { OTA_UPLOAD_START, OTA_UPLOAD_ERROR };

// Start a WiFi access point with a web upload page and ArduinoOTA.
// on_event is called from ota_loop() when an upload starts or fails;
// on success the board reboots on the new firmware.
void ota_begin(const char* network_name, void (*on_event)(OtaEvent));
void ota_end();
void ota_loop();
bool ota_active();
