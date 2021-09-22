#include <Arduino.h>
// #include <SparkFunDMX.h>
#include <WS2812FX.h>
// #include "ESP32_RMT_Driver.h"
#include <dmx.h>


#define LED_COUNT 600
#define LED_PIN 25

#define INCA_2

#define TOTAL_CHANNELS 512

#define DMX_OFF 0
// #define DMX_CHANNEL_ON        (DMX_OFF + 0)
#define DMX_CHANNEL_BRIGHT    (DMX_OFF + 1)
#define DMX_CHANNEL_COLOR_1_R (DMX_OFF + 2)
#define DMX_CHANNEL_COLOR_1_G (DMX_OFF + 3)
#define DMX_CHANNEL_COLOR_1_B (DMX_OFF + 4)
#define DMX_CHANNEL_SPEED     (DMX_OFF + 5)
#define DMX_CHANNEL_STROBE    (DMX_OFF + 6)

#define DMX_CHANNEL_MODE_0    (DMX_OFF + 7)
#define DMX_CHANNEL_MODE_1    (DMX_OFF + 8)
#define DMX_CHANNEL_MODE_2    (DMX_OFF + 9)
#define DMX_CHANNEL_MODE_3    (DMX_OFF + 10)
#define DMX_CHANNEL_MODE_4    (DMX_OFF + 11)

// SparkFunDMX dmx;
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800, 20, 20);

uint32_t color_1 = 0xFF0000;
uint16_t speed = 2000;

#ifdef INCA_1
	#define RAY_1  0, 0,  25
	#define RAY_2  1, 27, 52
	#define RAY_3  2, 54, 79

	#define EYE_C  3,   79, 111 // third eye
	#define TOP    4,  112, 212 // tete
	#define APP    5,  213, 218 // aprendice
	#define EYE_R  6,  219, 264 // eye right
	#define BOTTOM 7,  265, 348 // bottom  
	#define EYE_L  8,  349, 396 // eye left
	#define NOSE   9,  397, 461 // nose
	#define MOUTH  10, 462, 512 // mouth

	#define RAY_4  11, 514, 539 
	#define RAY_5  12, 540, 565
	#define RAY_6  13, 566, 591

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
#endif


void TwinkleFade() {
	ws2812fx.setSegment(RAY_1, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(RAY_2, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(RAY_3, FX_MODE_TWINKLE_FADE);

	ws2812fx.setSegment(EYE_C, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(TOP, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(APP, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(EYE_R, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(BOTTOM, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(EYE_L, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(NOSE, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(MOUTH, FX_MODE_TWINKLE_FADE);

	ws2812fx.setSegment(RAY_4, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(RAY_5, FX_MODE_TWINKLE_FADE);
	ws2812fx.setSegment(RAY_6, FX_MODE_TWINKLE_FADE);
}

void HyperSparkle() {
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
}

void test() {
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
}

void strobe() {
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
}

void static_anim() {
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
}

void hypno() {
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
}

void wipe() {
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
}

void rainbow() {
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
}

void outline() {
	ws2812fx.setSegment(TOP,    FX_MODE_RAINBOW_CYCLE);
	ws2812fx.setSegment(BOTTOM, FX_MODE_RAINBOW_CYCLE);  
	ws2812fx.setSegment(NOSE,   FX_MODE_RAINBOW_CYCLE);
}

void eye() {
	ws2812fx.setSegment(EYE_L, FX_MODE_BLINK, color_1, 50);
	ws2812fx.setSegment(EYE_R, FX_MODE_BLINK, color_1, 50);
	ws2812fx.setSegment(EYE_C, FX_MODE_BLINK, color_1, 50);

	ws2812fx.setSegment(RAY_1, FX_MODE_BLINK, color_1, 50);
	ws2812fx.setSegment(RAY_2, FX_MODE_BLINK, color_1, 50);
	ws2812fx.setSegment(RAY_3, FX_MODE_BLINK, color_1, 50);

	ws2812fx.setSegment(RAY_4, FX_MODE_BLINK, color_1, 50);
	ws2812fx.setSegment(RAY_5, FX_MODE_BLINK, color_1, 50);
	ws2812fx.setSegment(RAY_6, FX_MODE_BLINK, color_1, 50);
}

unsigned long timer = 0; // a timer
int anim = 1;
uint8_t bright = 255;

// Custom show functions which will use the RMT hardware to drive the LEDs.
// Need a separate function for each ws2812fx instance.
// void myCustomShow1(void) {
// 	uint8_t* pixels = ws2812fx.getPixels();
// 	// numBytes is one more then the size of the ws2812fx's *pixels array.
// 	// the extra byte is used by the driver to insert the LED reset pulse at the end.
// 	uint16_t numBytes = ws2812fx.getNumBytes() + 1;
// 	rmt_write_sample(RMT_CHANNEL_0, pixels, numBytes, false); // channel 0
// }


void DMX_task(void* parameter) {
	// Serial.printf("Task start\n");
	DMX::Initialize(input);
	Serial.println("initialized...");

	for (;;) { // infinite loop
		if (DMX::IsHealthy())     {

			int new_anim = 0;

			if (DMX::Read(DMX_CHANNEL_STROBE) > 127)
				new_anim = 6;
			else if (DMX::Read(DMX_CHANNEL_MODE_0) > 127)
				new_anim = 1;
			else if (DMX::Read(DMX_CHANNEL_MODE_1) > 127)
				new_anim = 2;
			else if (DMX::Read(DMX_CHANNEL_MODE_2) > 127)
				new_anim = 3;
			else if (DMX::Read(DMX_CHANNEL_MODE_3) > 127)
				new_anim = 4;
			else if (DMX::Read(DMX_CHANNEL_MODE_4) > 127)
				new_anim = 7;
			else
				new_anim = 0;
			

			if (new_anim != anim) {
				Serial.printf("New anim %d\n", new_anim);
				anim = new_anim;
				ws2812fx.resetSegments();
				ws2812fx.strip_off();
				switch (anim) {
					case 0:
						hypno();
						break;
					case 1:
						static_anim();
						break;
					case 2:
						wipe();
						break;
					case 3:
						TwinkleFade();
						break;
					case 4:
						eye();
						break;
					case 5:
						test();
						anim = -1;
						break;
					case 7:
						HyperSparkle();
						break;
					case 6:
						strobe();
						break;
				}
			}


			if (anim != 6) {
				color_1 = ((uint32_t)DMX::Read(DMX_CHANNEL_COLOR_1_R) << 16) | ((uint32_t)DMX::Read(DMX_CHANNEL_COLOR_1_G) << 8) | ((uint32_t)DMX::Read(DMX_CHANNEL_COLOR_1_B));
				Serial.printf("Color: %d, %d, %d", DMX::Read(DMX_CHANNEL_COLOR_1_R), DMX::Read(DMX_CHANNEL_COLOR_1_G), DMX::Read(DMX_CHANNEL_COLOR_1_B));
				ws2812fx.setAllColor(color_1);
			}

			Serial.printf(", Bright: %d", DMX::Read(DMX_CHANNEL_BRIGHT));
			if (bright != DMX::Read(DMX_CHANNEL_BRIGHT)) {
				bright = DMX::Read(DMX_CHANNEL_BRIGHT);
				ws2812fx.setBrightness(DMX::Read(DMX_CHANNEL_BRIGHT));
			}
			speed = DMX::Read(DMX_CHANNEL_SPEED) * 20;
			ws2812fx.setAllSpeed(speed);
			Serial.printf(", Speed: %d", speed);
			Serial.printf("\n");
		}

		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void led_task(void* parameter) {
	ws2812fx.init();
	ws2812fx.setBrightness(255);
	ws2812fx.start();
	for (;;) {
		ws2812fx.service();


		// unsigned long now = millis();
		// if ((now - timer) > 5000) { // every 10 seconds
		// 	Serial.println(anim);
		// 	ws2812fx.resetSegments();
		// 	ws2812fx.strip_off();
		// 	chang_anim();
		// 	timer = now;
		// 	anim++;
		// }
	vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}


void setup() {
	Serial.begin(115200);
	Serial.printf("Start\n");
	// rmt_tx_int(RMT_CHANNEL_0, ws2812fx.getPin()); // assign ws2812fx1 to RMT channel 0
	// ws2812fx.setCustomShow(myCustomShow1); // set the custom show function to forgo the NeoPixel


	// test();
	hypno();

	xTaskCreatePinnedToCore(
		led_task,    // Function that should be called
		"led_task",   // Name of the task (for debugging)
		10000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		2,               // Task priority
		NULL,             // Task handle
		1          // Core you want to run the task on (0 or 1)
	);
	xTaskCreatePinnedToCore(
		DMX_task,    // Function that should be called
		"DMX_task",   // Name of the task (for debugging)
		15000,            // Stack size (bytes)
		NULL,            // Parameter to pass
		1,               // Task priority
		NULL,             // Task handle
		0          // Core you want to run the task on (0 or 1)
	);
}

void loop() {
	vTaskDelay(1 / portTICK_PERIOD_MS);
}	