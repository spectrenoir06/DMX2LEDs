#include <Arduino.h>

#include <WS2812FX.h>
#include <dmx.h>

// #include <WiFi.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <AsyncElegantOTA.h>

#define TOTAL_CHANNELS 512

int ledPin_0 = 12;    // BUE
int ledPin_1 = 13;    // GREEN
int ledPin_2 = 5;     // RED

const int freq = 200000;
const int resolution = 8;

const int channel_red   = 2;
const int channel_green = 1;
const int channel_blue  = 0;

#define DMX_CHANNEL_COLOR_1_R (DMX_OFF + 1)
#define DMX_CHANNEL_COLOR_1_G (DMX_OFF + 2)
#define DMX_CHANNEL_COLOR_1_B (DMX_OFF + 3)
#define DMX_CHANNEL_SPEED     (DMX_OFF + 4)
#define DMX_CHANNEL_STROBE    (DMX_OFF + 5)
#define DMX_CHANNEL_STROBE_G  (DMX_OFF + 6)

#define DMX_CHANNEL_MODE_1    (DMX_OFF + 7)
#define DMX_CHANNEL_MODE_2    (DMX_OFF + 8)
#define DMX_CHANNEL_MODE_3    (DMX_OFF + 9)
#define DMX_CHANNEL_MODE_4    (DMX_OFF + 10)
#define DMX_CHANNEL_MODE_5    (DMX_OFF + 11)
#define DMX_CHANNEL_MODE_6    (DMX_OFF + 12)
#define DMX_CHANNEL_MODE_7    (DMX_OFF + 13)
#define DMX_CHANNEL_MODE_8    (DMX_OFF + 14)
#define DMX_CHANNEL_MODE_9    (DMX_OFF + 15)

#define DMX_CHANNEL_BRIGHT    (DMX_OFF + 16)
#define DMX_CHANNEL_STROBE_SPEED (DMX_OFF + 17)


WS2812FX ws2812fx    = WS2812FX(LED_COUNT, LED_PIN, LEDS_TYPE, 20, 20);
WS2812FX ws2812fx_p  = WS2812FX(LED_COUNT, LED_PIN, LEDS_TYPE, 1, 1);

// AsyncWebServer server(80);

uint32_t color_1 = 0xFF0000;
int32_t speed = 0;
int32_t strobe_speed =0;
uint8_t is_strobe = 0;

#if defined(INCA_1)
	#define RAY_1  0, 0,  23
	#define RAY_2  1, 27, 50
	#define RAY_3  2, 54, 77

	#define EYE_C  3,   79, 111 // third eye
	#define TOP    4,  112, 212 // tete
	#define APP    5,  213, 218 // aprendice
	#define EYE_R  6,  219, 264 // eye right
	#define BOTTOM 7,  265, 348 // bottom  
	#define EYE_L  8,  349, 396 // eye left
	#define NOSE   9,  397, 461 // nose
	#define MOUTH  10, 462, 512 // mouth

	#define RAY_4  11, 513, 538 
	#define RAY_5  12, 539, 564
	#define RAY_6  13, 565, 590

#elif defined(INCA_2)
	#define RAY_1  0, 0,  25
	#define RAY_2  1, 25, 50
	#define RAY_3  2, 51, 76

	#define EYE_C  3,  78-1, 111-1    // third eye
	#define TOP    4, 112-1, 205-1  // tete
	#define APP    5, 206-1, 209-1 // aprendice
	#define EYE_R  6, 210-1, 256-1 // eye right
	#define BOTTOM 7, 257-1, 341-1 // bottom  
	#define EYE_L  8, 342-1, 387-1 // eye left
	#define NOSE   9, 388-1, 447-1 // nose
	#define MOUTH 10, 448-1, 500-1 // mouth

	#define RAY_4  11, 501-1, 526-1 
	#define RAY_5  12, 527-1, 552-1
	#define RAY_6  13, 553-1, 578-1

#elif defined(INCA_3)
	#define RAY_1  0, 0,  25
	#define RAY_2  1, 25, 50
	#define RAY_3  2, 51, 76

	#define EYE_C  3,  78-1, 111-1    // third eye
	#define TOP    4, 112-1, 205-1  // tete
	#define APP    5, 206-1, 209-1 // aprendice
	#define EYE_R  6, 210-1, 256-1 // eye right
	#define BOTTOM 7, 257-1, 341-1 // bottom  
	#define EYE_L  8, 342-1, 387-1 // eye left
	#define NOSE   9, 388-1, 447-1 // nose
	#define MOUTH 10, 448-1, 500-1 // mouth

	#define RAY_4  11, 501-1, 526-1 
	#define RAY_5  12, 527-1, 552-1
	#define RAY_6  13, 553-1, 578-1

#elif defined(LOGO)
	#define RAY_1  0, 1, 143

#elif defined(trendy_1)
	#define RAY_1  0, 0, (LED_COUNT-1)	
#elif defined(trendy_2)
	#define RAY_1  0, 0, (LED_COUNT-1)
#endif

void firework() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_FIREWORKS);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(RAY_1, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(RAY_2, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(RAY_3, FX_MODE_FIREWORKS);

		ws2812fx.setSegment(EYE_C, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(TOP, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(APP, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(EYE_R, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(BOTTOM, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(EYE_L, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(NOSE, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(MOUTH, FX_MODE_FIREWORKS);

		ws2812fx.setSegment(RAY_4, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(RAY_5, FX_MODE_FIREWORKS);
		ws2812fx.setSegment(RAY_6, FX_MODE_FIREWORKS);
	#endif
}

void firework_rainbow() {
#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_FIREWORKS_RANDOM);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(RAY_1, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(RAY_2, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(RAY_3, FX_MODE_FIREWORKS_RANDOM);

		ws2812fx.setSegment(EYE_C, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(TOP, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(APP, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(EYE_R, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(BOTTOM, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(EYE_L, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(NOSE, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(MOUTH, FX_MODE_FIREWORKS_RANDOM);

		ws2812fx.setSegment(RAY_4, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(RAY_5, FX_MODE_FIREWORKS_RANDOM);
		ws2812fx.setSegment(RAY_6, FX_MODE_FIREWORKS_RANDOM);
	#endif
}

void HyperSparkle() {
#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_HYPER_SPARKLE);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(RAY_1, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(RAY_2, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(RAY_3, FX_MODE_HYPER_SPARKLE);

		ws2812fx.setSegment(EYE_C, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(TOP, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(APP, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(EYE_R, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(BOTTOM, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(EYE_L, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(NOSE, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(MOUTH, FX_MODE_HYPER_SPARKLE);

		ws2812fx.setSegment(RAY_4, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(RAY_5, FX_MODE_HYPER_SPARKLE);
		ws2812fx.setSegment(RAY_6, FX_MODE_HYPER_SPARKLE);
	#endif
}

void test() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
	#else
		ws2812fx.setSegment(RAY_1,  FX_MODE_STATIC, RED);
		ws2812fx.setSegment(RAY_2,  FX_MODE_STATIC, GREEN);
		ws2812fx.setSegment(RAY_3,  FX_MODE_STATIC, BLUE);

		ws2812fx.setSegment(EYE_C,  FX_MODE_STATIC, YELLOW);
		ws2812fx.setSegment(TOP,    FX_MODE_STATIC, MAGENTA);
		ws2812fx.setSegment(APP,    FX_MODE_STATIC, CYAN);
		ws2812fx.setSegment(EYE_R,  FX_MODE_STATIC, RED);
		ws2812fx.setSegment(BOTTOM, FX_MODE_STATIC, GREEN);
		ws2812fx.setSegment(EYE_L,  FX_MODE_STATIC, BLUE);
		ws2812fx.setSegment(NOSE,   FX_MODE_STATIC, YELLOW);
		ws2812fx.setSegment(MOUTH,  FX_MODE_STATIC, MAGENTA);
		
		ws2812fx.setSegment(RAY_4,  FX_MODE_STATIC, RED);
		ws2812fx.setSegment(RAY_5,  FX_MODE_STATIC, GREEN);
		ws2812fx.setSegment(RAY_6,  FX_MODE_STATIC, BLUE);
	#endif
}

void strobe() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_BLINK, WHITE, 200);
	#else
		ws2812fx.setSegment(RAY_1, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(RAY_2, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(RAY_3, FX_MODE_BLINK, WHITE, 200);

		ws2812fx.setSegment(EYE_C, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(TOP, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(APP, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(EYE_R, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(BOTTOM, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(EYE_L, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(NOSE, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(MOUTH, FX_MODE_BLINK, WHITE, 200);

		ws2812fx.setSegment(RAY_4, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(RAY_5, FX_MODE_BLINK, WHITE, 200);
		ws2812fx.setSegment(RAY_6, FX_MODE_BLINK, WHITE, 200);
	#endif
}

void static_anim() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(RAY_1, FX_MODE_STATIC);
		ws2812fx.setSegment(RAY_2, FX_MODE_STATIC);
		ws2812fx.setSegment(RAY_3, FX_MODE_STATIC);

		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
		ws2812fx.setSegment(TOP, FX_MODE_STATIC);
		ws2812fx.setSegment(APP, FX_MODE_STATIC);
		ws2812fx.setSegment(EYE_R, FX_MODE_STATIC);
		ws2812fx.setSegment(BOTTOM, FX_MODE_STATIC);
		ws2812fx.setSegment(EYE_L, FX_MODE_STATIC);
		ws2812fx.setSegment(NOSE, FX_MODE_STATIC);
		ws2812fx.setSegment(MOUTH, FX_MODE_STATIC);

		ws2812fx.setSegment(RAY_4, FX_MODE_STATIC);
		ws2812fx.setSegment(RAY_5, FX_MODE_STATIC);
		ws2812fx.setSegment(RAY_6, FX_MODE_STATIC);
	#endif
}

void hypno() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_THEATER_CHASE, color_1, 50);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(RAY_1, FX_MODE_THEATER_CHASE, color_1, 50);
		ws2812fx.setSegment(RAY_2, FX_MODE_THEATER_CHASE, color_1, 50);
		ws2812fx.setSegment(RAY_3, FX_MODE_THEATER_CHASE, color_1, 50);

		ws2812fx.setSegment(RAY_4, FX_MODE_THEATER_CHASE, color_1, 50);
		ws2812fx.setSegment(RAY_5, FX_MODE_THEATER_CHASE, color_1, 50);
		ws2812fx.setSegment(RAY_6, FX_MODE_THEATER_CHASE, color_1, 50);

		ws2812fx.setSegment(EYE_C, FX_MODE_THEATER_CHASE, color_1, 50);
		ws2812fx.setSegment(EYE_L, FX_MODE_THEATER_CHASE, color_1, 50);
		ws2812fx.setSegment(EYE_R, FX_MODE_THEATER_CHASE, color_1, 50);
		ws2812fx.setSegment(MOUTH, FX_MODE_THEATER_CHASE, color_1, 50);
	#endif	
}

void wipe() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_COLOR_WIPE, color_1, 3000);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(RAY_1, FX_MODE_COLOR_WIPE, color_1, 3000);
		ws2812fx.setSegment(RAY_2, FX_MODE_COLOR_WIPE, color_1, 3000);
		ws2812fx.setSegment(RAY_3, FX_MODE_COLOR_WIPE, color_1, 3000);

		ws2812fx.setSegment(RAY_4, FX_MODE_COLOR_WIPE, color_1, 3000);
		ws2812fx.setSegment(RAY_5, FX_MODE_COLOR_WIPE, color_1, 3000);
		ws2812fx.setSegment(RAY_6, FX_MODE_COLOR_WIPE, color_1, 3000);

		ws2812fx.setSegment(EYE_C,  FX_MODE_FADE);
		ws2812fx.setSegment(TOP,    FX_MODE_FADE);
		ws2812fx.setSegment(APP,    FX_MODE_FADE);
		ws2812fx.setSegment(EYE_R,  FX_MODE_FADE);
		ws2812fx.setSegment(BOTTOM, FX_MODE_FADE);
		ws2812fx.setSegment(EYE_L,  FX_MODE_FADE);
		ws2812fx.setSegment(NOSE,   FX_MODE_FADE);
		ws2812fx.setSegment(MOUTH,  FX_MODE_FADE);
	#endif
}

void rainbow() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_RAINBOW, color_1, 100);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(EYE_C, FX_MODE_RAINBOW, color_1, 100);
		ws2812fx.setSegment(EYE_L, FX_MODE_RAINBOW, color_1, 100);
		ws2812fx.setSegment(EYE_R, FX_MODE_RAINBOW, color_1, 100);
		ws2812fx.setSegment(MOUTH, FX_MODE_RAINBOW);

		ws2812fx.setSegment(TOP,    FX_MODE_RAINBOW, color_1, 300);
		ws2812fx.setSegment(BOTTOM, FX_MODE_RAINBOW, color_1, 300);
		ws2812fx.setSegment(NOSE,   FX_MODE_RAINBOW, color_1, 300);

		ws2812fx.setSegment(RAY_1, FX_MODE_RAINBOW, color_1, 10);
		ws2812fx.setSegment(RAY_2, FX_MODE_RAINBOW, color_1, 10);
		ws2812fx.setSegment(RAY_3, FX_MODE_RAINBOW, color_1, 10);

		ws2812fx.setSegment(RAY_4, FX_MODE_RAINBOW, color_1, 10);
		ws2812fx.setSegment(RAY_5, FX_MODE_RAINBOW, color_1, 10);
		ws2812fx.setSegment(RAY_6, FX_MODE_RAINBOW, color_1, 10);
	#endif
}

void outline() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_RAINBOW_CYCLE);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(TOP,    FX_MODE_RAINBOW_CYCLE);
		ws2812fx.setSegment(BOTTOM, FX_MODE_RAINBOW_CYCLE);  
		ws2812fx.setSegment(NOSE,   FX_MODE_RAINBOW_CYCLE);
	#endif
}

void strobe_color() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_BLINK, color_1, 200);
	#else
		ws2812fx.setSegment(EYE_L, FX_MODE_BLINK, color_1, 200);
		ws2812fx.setSegment(EYE_R, FX_MODE_BLINK, color_1, 200);
		ws2812fx.setSegment(EYE_C, FX_MODE_BLINK, color_1, 200);

		ws2812fx.setSegment(RAY_1, FX_MODE_BLINK, color_1, 200);
		ws2812fx.setSegment(RAY_2, FX_MODE_BLINK, color_1, 200);
		ws2812fx.setSegment(RAY_3, FX_MODE_BLINK, color_1, 200);

		ws2812fx.setSegment(RAY_4, FX_MODE_BLINK, color_1, 200);
		ws2812fx.setSegment(RAY_5, FX_MODE_BLINK, color_1, 200);
		ws2812fx.setSegment(RAY_6, FX_MODE_BLINK, color_1, 200);

		ws2812fx.setSegment(TOP,   FX_MODE_BLINK, color_1, 200);
		ws2812fx.setSegment(BOTTOM,FX_MODE_BLINK, color_1, 200);
		ws2812fx.setSegment(NOSE,  FX_MODE_BLINK, color_1, 200);
	#endif
}


void chase_rainbow() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_CHASE_RAINBOW);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(EYE_L, FX_MODE_CHASE_RAINBOW);
		ws2812fx.setSegment(EYE_R, FX_MODE_CHASE_RAINBOW);
		ws2812fx.setSegment(EYE_C, FX_MODE_CHASE_RAINBOW);

		ws2812fx.setSegment(RAY_1, FX_MODE_CHASE_RAINBOW);
		ws2812fx.setSegment(RAY_2, FX_MODE_CHASE_RAINBOW);
		ws2812fx.setSegment(RAY_3, FX_MODE_CHASE_RAINBOW);

		ws2812fx.setSegment(RAY_4, FX_MODE_CHASE_RAINBOW);
		ws2812fx.setSegment(RAY_5, FX_MODE_CHASE_RAINBOW);
		ws2812fx.setSegment(RAY_6, FX_MODE_CHASE_RAINBOW);

		ws2812fx.setSegment(TOP, FX_MODE_CHASE_RAINBOW);
		ws2812fx.setSegment(BOTTOM, FX_MODE_CHASE_RAINBOW);
		ws2812fx.setSegment(NOSE, FX_MODE_CHASE_RAINBOW);
	#endif
}

void running_light() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_RUNNING_LIGHTS);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(EYE_L, FX_MODE_RUNNING_LIGHTS);
		ws2812fx.setSegment(EYE_R, FX_MODE_RUNNING_LIGHTS);
		ws2812fx.setSegment(EYE_C, FX_MODE_RUNNING_LIGHTS);

		ws2812fx.setSegment(RAY_1, FX_MODE_RUNNING_LIGHTS);
		ws2812fx.setSegment(RAY_2, FX_MODE_RUNNING_LIGHTS);
		ws2812fx.setSegment(RAY_3, FX_MODE_RUNNING_LIGHTS);

		ws2812fx.setSegment(RAY_4, FX_MODE_RUNNING_LIGHTS);
		ws2812fx.setSegment(RAY_5, FX_MODE_RUNNING_LIGHTS);
		ws2812fx.setSegment(RAY_6, FX_MODE_RUNNING_LIGHTS);

		ws2812fx.setSegment(TOP, FX_MODE_RUNNING_LIGHTS);
		ws2812fx.setSegment(BOTTOM, FX_MODE_RUNNING_LIGHTS);
		ws2812fx.setSegment(NOSE, FX_MODE_RUNNING_LIGHTS);
	#endif
}

void comete() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_COMET);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(EYE_L, FX_MODE_COMET);
		ws2812fx.setSegment(EYE_R, FX_MODE_COMET);
		ws2812fx.setSegment(EYE_C, FX_MODE_COMET);

		ws2812fx.setSegment(RAY_1, FX_MODE_COLOR_WIPE_REV);
		ws2812fx.setSegment(RAY_2, FX_MODE_COLOR_WIPE_REV);
		ws2812fx.setSegment(RAY_3, FX_MODE_COLOR_WIPE_REV);

		ws2812fx.setSegment(RAY_4, FX_MODE_COLOR_WIPE_REV);
		ws2812fx.setSegment(RAY_5, FX_MODE_COLOR_WIPE_REV);
		ws2812fx.setSegment(RAY_6, FX_MODE_COLOR_WIPE_REV);

		ws2812fx.setSegment(TOP,    FX_MODE_FADE);
		ws2812fx.setSegment(BOTTOM, FX_MODE_FADE);
		ws2812fx.setSegment(NOSE,   FX_MODE_FADE);
	#endif
}

void TwinkleFOX() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_TWINKLEFOX);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(EYE_L, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(EYE_R, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(EYE_C, FX_MODE_TWINKLEFOX);

		ws2812fx.setSegment(RAY_1, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(RAY_2, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(RAY_3, FX_MODE_TWINKLEFOX);

		ws2812fx.setSegment(RAY_4, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(RAY_5, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(RAY_6, FX_MODE_TWINKLEFOX);

		ws2812fx.setSegment(TOP, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(BOTTOM, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(NOSE, FX_MODE_TWINKLEFOX);
	#endif
}

void fire() {
	#if defined(LOGO) or defined(trendy_1) or defined(trendy_2)
		ws2812fx.setSegment(RAY_1, FX_MODE_TWINKLEFOX);
	#elif defined(INCA_3)
		ws2812fx.setSegment(EYE_C, FX_MODE_STATIC);
	#else
		ws2812fx.setSegment(EYE_L, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(EYE_R, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(EYE_C, FX_MODE_TWINKLEFOX);

		ws2812fx.setSegment(RAY_1, FX_MODE_CHASE_COLOR);
		ws2812fx.setSegment(RAY_2, FX_MODE_CHASE_COLOR);
		ws2812fx.setSegment(RAY_3, FX_MODE_CHASE_COLOR);

		ws2812fx.setSegment(RAY_4, FX_MODE_CHASE_COLOR);
		ws2812fx.setSegment(RAY_5, FX_MODE_CHASE_COLOR);
		ws2812fx.setSegment(RAY_6, FX_MODE_CHASE_COLOR);

		ws2812fx.setSegment(TOP, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(BOTTOM, FX_MODE_TWINKLEFOX);
		ws2812fx.setSegment(NOSE, FX_MODE_TWINKLEFOX);
	#endif
}

int anim = 0;
uint8_t bright = 255;
uint8_t blackout = 0;

void DMX_task(void* parameter) {
	// Serial.printf("Task start\n");
	DMX::Initialize(input);
	Serial.println("initialized...");
	wipe();

	for (;;) { // infinite loop
		if (DMX::IsHealthy())     {

			int new_anim = anim;


			if (DMX::Read(DMX_CHANNEL_STROBE_G) > 127) {
				is_strobe = 1;
			} else {
				blackout = 0;
				is_strobe = 0;
			}

			if (DMX::Read(DMX_CHANNEL_STROBE) > 127)
				new_anim = 255;
			else if (DMX::Read(DMX_CHANNEL_MODE_1) > 127)
				new_anim = 1;
			else if (DMX::Read(DMX_CHANNEL_MODE_2) > 127)
				new_anim = 2;
			else if (DMX::Read(DMX_CHANNEL_MODE_3) > 127)
				new_anim = 3;
			else if (DMX::Read(DMX_CHANNEL_MODE_4) > 127)
				new_anim = 4;
			else if (DMX::Read(DMX_CHANNEL_MODE_5) > 127)
				new_anim = 5;
			else if (DMX::Read(DMX_CHANNEL_MODE_6) > 127)
				new_anim = 6;
			else if (DMX::Read(DMX_CHANNEL_MODE_7) > 127)
				new_anim = 7;
			else if (DMX::Read(DMX_CHANNEL_MODE_8) > 127)
				new_anim = 8;
			else if (DMX::Read(DMX_CHANNEL_MODE_9) > 127)
				new_anim = 9;
			else
				new_anim = 254;
			

			if (new_anim != anim) {
				Serial.printf("New anim %d\n", new_anim);
				anim = new_anim;
				ws2812fx.resetSegments();
				ws2812fx.strip_off();
				switch (anim) {
					// case 0:
					// 	strobe_color();
					// 	break;
					case 1:
						wipe();
						break;
					case 2:
						hypno();
						break;
					case 3:
						firework();
						break;
					case 4:
						running_light();
						break;
					case 5:
						TwinkleFOX();
						break;
					case 6:
						fire();
						break;
					case 7:
						comete();
						break;
					case 8:
						firework_rainbow();
						break;
					case 9:
						chase_rainbow();
						break;
					case 255:
						strobe();
						break;
					case 254:
						static_anim();
						break;
				}
				ws2812fx.setAllSpeed(speed);
			}

			Serial.printf("Color: %d, %d, %d", DMX::Read(DMX_CHANNEL_COLOR_1_R), DMX::Read(DMX_CHANNEL_COLOR_1_G), DMX::Read(DMX_CHANNEL_COLOR_1_B));
			if (anim != 255 && anim != 0) {
				uint32_t new_color = ((uint32_t)DMX::Read(DMX_CHANNEL_COLOR_1_R) << 16) | ((uint32_t)DMX::Read(DMX_CHANNEL_COLOR_1_G) << 8) | ((uint32_t)DMX::Read(DMX_CHANNEL_COLOR_1_B));
				if (new_color != color_1) {
					color_1 = new_color;
					if (new_anim == 0) // strobe color hack
						strobe_color();
				}
				ws2812fx.setAllColor(color_1);
			}

			#ifdef USE_BRIGHT
				Serial.printf(", Bright: %d", DMX::Read(DMX_CHANNEL_BRIGHT));
				if (bright != DMX::Read(DMX_CHANNEL_BRIGHT)) {
					bright = DMX::Read(DMX_CHANNEL_BRIGHT);
				}
			#endif

			if (anim == 254) { // static 
				speed = 0;
			} else {
				int new_speed = 2650 - DMX::Read(DMX_CHANNEL_SPEED) * 10;
				if (speed != new_speed) {
					speed = new_speed;
					ws2812fx.setAllSpeed(speed);
				}
			}

			int new_speed = 255 - DMX::Read(DMX_CHANNEL_STROBE_SPEED);
			new_speed = map(new_speed, 0, 255, 200, 1000);
			if (strobe_speed != new_speed) {
				strobe_speed = new_speed;
			}

			Serial.printf(", Speed: %d", speed);
			Serial.printf(", anime = %d", anim);
			Serial.printf("\n");
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

#ifdef INCA_3
	void myCustomShow(void) {
		uint8_t* pixels_v1 = ws2812fx.getPixels();
		ledcWrite(channel_red,   (pixels_v1[77*3 + 1] * bright) >> 8);
		ledcWrite(channel_green, (pixels_v1[77*3 + 0] * bright) >> 8);
		ledcWrite(channel_blue,  (pixels_v1[77*3 + 2] * bright) >> 8);
	}
#else
	void myCustomShow(void) {
		uint8_t* pixels_v1 = ws2812fx.getPixels();
		uint8_t* pixels_p = ws2812fx_p.getPixels();

		for (int i = 0; i < ws2812fx_p.getNumBytes(); i++) {
			uint32_t p = pixels_v1[i];
			if (blackout)
				p = 0;
			else
				p = (p * bright) >> 8;
			pixels_p[i] = p;
		}
		ws2812fx_p.Adafruit_NeoPixel::show();
	}
#endif


void led_task(void* parameter) {
	ws2812fx_p.init();

	ws2812fx.init();
	ws2812fx.setBrightness(255);
	ws2812fx.start();

	#ifdef INCA_3
		ledcSetup(0, freq, resolution);
		ledcSetup(1, freq, resolution);
		ledcSetup(2, freq, resolution);
		ledcAttachPin(ledPin_0, channel_blue);
		ledcAttachPin(ledPin_1, channel_green);
		ledcAttachPin(ledPin_2, channel_red);
	#endif
	ws2812fx.setCustomShow(myCustomShow); // set the custom show function to forgo the NeoPixel
	int ctn= 0;
	for (;;) {
		ws2812fx.service();
		if (is_strobe) {
			ctn++;
			// Serial.printf("%d\n", ctn);
			if (ctn > ((strobe_speed) / 100)) {
				blackout = !blackout;
				ctn = 0;
			}
		}
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}

// void strobe_task(void* parameter) {
// 	int ctn = 0;
// 	for (;;) {

// 		vTaskDelay(1 / portTICK_PERIOD_MS);
// 	}
// }


void setup() {
	Serial.begin(115200);
	Serial.printf("Start\n");

	// test();

	// Serial.println("Setting AP (Access Point)");
	// WiFi.softAP(AP_SSID, AP_PASSWORD);
	// IPAddress IP = WiFi.softAPIP();
	// Serial.print("AP IP address: ");
	// Serial.println(IP);

	// AsyncElegantOTA.begin(&server);
	// server.begin();
	// Serial.println("HTTP server started");

	xTaskCreatePinnedToCore(
		led_task,    // Function that should be called
		"led_task",   // Name of the task (for debugging)
		10000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		1,               // Task priority
		NULL,             // Task handle
		0          // Core you want to run the task on (0 or 1)
	);
	// xTaskCreatePinnedToCore(
	// 	strobe_task,    // Function that should be called
	// 	"strobe_task",   // Name of the task (for debugging)
	// 	10000,            // Stack size (bytes)
	// 	NULL,            // Parameter to pass
	// 	1,               // Task priority
	// 	NULL,             // Task handle
	// 	1          // Core you want to run the task on (0 or 1)
	// );
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
	vTaskDelay(1 / portTICK_PERIOD_MS);
}	