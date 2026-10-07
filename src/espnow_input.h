// DMX and WizMote remote over ESP-NOW, on when DIP switch 10 is ON at power-up.
// See espnow_input.cpp.
#pragma once

#include <stdint.h>

#define ESPNOW_CHANNEL 1 // WiFi channel of dmx2espnow and of the WizMote

// WizMote buttons
#define WIZMOTE_ON          1
#define WIZMOTE_OFF         2
#define WIZMOTE_NIGHT       3
#define WIZMOTE_BRIGHT_DOWN 8
#define WIZMOTE_BRIGHT_UP   9
#define WIZMOTE_ONE         16
#define WIZMOTE_TWO         17
#define WIZMOTE_THREE       18
#define WIZMOTE_FOUR        19

void espnow_begin();
bool espnow_enabled();                     // started by espnow_begin()
void espnow_resume();                      // after the WiFi update mode, which changes the WiFi mode
bool espnow_dmx_healthy();                 // a DMX packet received in the last 500 ms
uint8_t espnow_dmx_read(uint16_t channel); // 1-512, 0 for channels the sender doesn't send
int espnow_take_button();                  // next WizMote button pressed, -1 if none
void espnow_set_pairing(bool on);          // pairing mode: senders heard are paired, packets not applied
void espnow_poll();                        // from DMX_task: saves pairings, logs refused senders
