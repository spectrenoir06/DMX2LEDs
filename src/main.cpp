#include <Arduino.h>
#include <WS2812FX.h>

#define LED_COUNT 600
#define LED_PIN 25

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800, 20, 20);

#define INCA_1

#ifdef INCA_1
	#define RAY_1  0, 0,  25
	#define RAY_2  1, 26, 51
	#define RAY_3  2, 52, 77

	#define EYE_C  3,   78, 111 // third eye
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
#else
	#define RAY_1  0, 0,  25
	#define RAY_2  1, 26, 51
	#define RAY_3  2, 52, 77

	#define EYE_C  3,  78, 111    // third eye
	#define TOP    4, 112, 205  // tete
	#define APP    5, 206, 209 // aprendice
	#define EYE_R  6, 210, 256 // eye right
	#define BOTTOM 7, 257, 341 // bottom  
	#define EYE_L  8, 342, 387 // eye left
	#define NOSE   9, 388, 447 // nose
	#define MOUTH 10, 448, 500 // mouth


	#define RAY_4  11, 501, 526 
	#define RAY_5  12, 527, 552
	#define RAY_6  13, 553, 579
#endif

void test() {
	ws2812fx.setSegment(RAY_1, FX_MODE_RAINBOW_CYCLE, RED);    // ray_1
	ws2812fx.setSegment(RAY_2, FX_MODE_RAINBOW_CYCLE, GREEN);  // ray_2
	ws2812fx.setSegment(RAY_3, FX_MODE_RAINBOW_CYCLE, BLUE);   // ray_3


	ws2812fx.setSegment(EYE_C,  FX_MODE_THEATER_CHASE_RAINBOW, ws2812fx.color_wheel(ws2812fx.random8()));  // third eye
	ws2812fx.setSegment(TOP,    FX_MODE_STATIC, ws2812fx.color_wheel(ws2812fx.random8())); // tete
	ws2812fx.setSegment(APP,    FX_MODE_STATIC, RED); // aprendice
	ws2812fx.setSegment(EYE_R,  FX_MODE_STATIC, ws2812fx.color_wheel(ws2812fx.random8()));  // eye right
	ws2812fx.setSegment(BOTTOM, FX_MODE_STATIC, ws2812fx.color_wheel(ws2812fx.random8())); // bottom  
	ws2812fx.setSegment(EYE_L,  FX_MODE_STATIC, ws2812fx.color_wheel(ws2812fx.random8()));  // eye left
	ws2812fx.setSegment(NOSE,   FX_MODE_STATIC, ws2812fx.color_wheel(ws2812fx.random8())); // nose
	ws2812fx.setSegment(MOUTH,  FX_MODE_STATIC, ws2812fx.color_wheel(ws2812fx.random8()));  // mouth
	
	ws2812fx.setSegment(RAY_4, FX_MODE_STATIC, ws2812fx.color_wheel(ws2812fx.random8()));
	ws2812fx.setSegment(RAY_5, FX_MODE_STATIC, ws2812fx.color_wheel(ws2812fx.random8()), 2000, REVERSE);
	ws2812fx.setSegment(RAY_6, FX_MODE_STATIC, ws2812fx.color_wheel(ws2812fx.random8()));
}

void hypno() {

	ws2812fx.setSegment(RAY_1, FX_MODE_THEATER_CHASE_RAINBOW, RED);  // ray_1
	ws2812fx.setSegment(RAY_2, FX_MODE_THEATER_CHASE_RAINBOW, RED, 2000, REVERSE);  // ray_2
	ws2812fx.setSegment(RAY_3, FX_MODE_THEATER_CHASE_RAINBOW, RED);  // ray_3

	ws2812fx.setSegment(RAY_4, FX_MODE_THEATER_CHASE_RAINBOW, RED);
	ws2812fx.setSegment(RAY_5, FX_MODE_THEATER_CHASE_RAINBOW, RED, 2000, REVERSE);
	ws2812fx.setSegment(RAY_6, FX_MODE_THEATER_CHASE_RAINBOW, RED);

	ws2812fx.setSegment(EYE_C, FX_MODE_THEATER_CHASE_RAINBOW, RED, 100);   // third eye
	ws2812fx.setSegment(EYE_L, FX_MODE_THEATER_CHASE_RAINBOW, RED, 100);  // eye left
	ws2812fx.setSegment(EYE_R, FX_MODE_THEATER_CHASE_RAINBOW, RED, 100);   // eye right
	ws2812fx.setSegment(MOUTH, FX_MODE_THEATER_CHASE_RAINBOW, RED, 100); // mouth
}

void rainbow() {
	ws2812fx.setSegment(EYE_C, FX_MODE_RAINBOW);  // third eye
	ws2812fx.setSegment(EYE_L, FX_MODE_RAINBOW);  // eye left
	ws2812fx.setSegment(EYE_R, FX_MODE_RAINBOW);  // eye right
	ws2812fx.setSegment(MOUTH, FX_MODE_RAINBOW);  // mouth

	ws2812fx.setSegment(TOP,    FX_MODE_RAINBOW); // tete
	ws2812fx.setSegment(BOTTOM, FX_MODE_RAINBOW); // bottom
	ws2812fx.setSegment(NOSE,   FX_MODE_RAINBOW); // nose
}

void outline() {
	ws2812fx.setSegment(TOP,    FX_MODE_RAINBOW); // tete
	ws2812fx.setSegment(BOTTOM, FX_MODE_RAINBOW); // bottom  
	ws2812fx.setSegment(NOSE,   FX_MODE_RAINBOW); // nose
}

void eye(uint32_t c, uint16_t speed) {
	ws2812fx.setSegment(EYE_L, FX_MODE_BLINK, c, speed);  // eye left
	ws2812fx.setSegment(EYE_R, FX_MODE_BLINK, c, speed);  // eye right
}

unsigned long timer = 0; // a timer
int anim = 1;


void setup() {
	ws2812fx.init();
	ws2812fx.setBrightness(255);
	test();
	// hypno();

	ws2812fx.start();
}

void loop() {
	unsigned long now = millis();
	ws2812fx.service();

	// if ((now - timer) > 5000) { // every 10 seconds
	// 	ws2812fx.resetSegments();
	// 	ws2812fx.strip_off();
	// 	switch (anim) {
	// 		case 0:
	// 			hypno();
	// 			break;
	// 		case 1:
	// 			rainbow();
	// 			break;
	// 		case 2:
	// 			outline();
	// 			break;
	// 		case 3:
	// 			eye(DIM(ws2812fx.color_wheel(ws2812fx.random8())), 100);
	// 			break;
	// 		case 4:
	// 			anim = -1;
	// 			break;
	// 	}
	// 	timer = now;
	// 	anim++;
	// }
}	