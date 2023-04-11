#include <Arduino.h>

#include <WS2812FX.h>
#include <dmx.h>

#define TOTAL_CHANNELS 512

uint8_t led_blink = 0;

#define DMX_CHANNEL_COLOR_1_R    ( 1)
#define DMX_CHANNEL_COLOR_1_G    ( 2)
#define DMX_CHANNEL_COLOR_1_B    ( 3)
#define DMX_CHANNEL_BRIGHT       ( 4)
#define DMX_CHANNEL_SPEED        ( 5)
#define DMX_CHANNEL_STROBE_SPEED ( 6)

#define DMX_CHANNEL_STROBE_G     ( 7)
#define DMX_CHANNEL_STROBE       ( 8)
#define DMX_CHANNEL_MODE_1       ( 9)
#define DMX_CHANNEL_MODE_2       (10)
#define DMX_CHANNEL_MODE_3       (11)
#define DMX_CHANNEL_MODE_4       (12)

#define DMX_CHANNEL_MODE_5       (13)
#define DMX_CHANNEL_MODE_6       (14)
#define DMX_CHANNEL_MODE_7       (15)
#define DMX_CHANNEL_MODE_8       (16)
#define DMX_CHANNEL_MODE_9       (17)

#define DMX_CHANNEL_LITE_ANIM    ( 8)

WS2812FX ws2812fx    = WS2812FX(LED_COUNT, LED_PIN_1, LEDS_TYPE, 20, 20);
WS2812FX ws2812fx_p  = WS2812FX(LED_COUNT, LED_PIN_1, LEDS_TYPE, 1, 1);

uint32_t color_1 = 0xFF0000;
int32_t  speed = 0;
int32_t  strobe_speed =0;
uint8_t  is_strobe = 0;
uint32_t strob_ctn = 0;
unsigned long lastUpdate = millis();


uint16_t snake(void) { // random chase
	static uint8_t snake_ctn = 0;
	static uint8_t snake_on = 0;
	WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment 

	for (uint16_t i = seg->stop; i > seg->start; i--)
		ws2812fx.setPixelColor(i, ws2812fx.getPixelColor(i - 1));

	snake_ctn++;
	if (snake_ctn > 2) {
		snake_on = !snake_on;
		snake_ctn = 0;
	}

	if (snake_on)
		ws2812fx.setPixelColor(seg->start, seg->colors[0]);
	else
		ws2812fx.setPixelColor(seg->start, 0, 0, 0);

	return (seg->speed / 25); // return the delay until the next animation step (in msec)
}


int snake_size = 10;


uint16_t snake2(void) { // random chase
	static uint32_t pos_x = 0;
	WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment

	for (int i = seg->start; i <= seg->stop; i++) // clear
		ws2812fx.setPixelColor(i, 0);

	for (int i = pos_x; i < pos_x + snake_size; i++)
		ws2812fx.setPixelColor(i, color_1);

	pos_x++;
	if (pos_x > seg->stop + snake_size)
		pos_x = 0;
	return (seg->speed / 25); // return the delay until the next animation step (in msec)
}

uint8_t snakeMode = ws2812fx.setCustomMode(F("snake mode"), snake);
uint8_t snakeMode2 = ws2812fx.setCustomMode(F("snake mode 2"), snake2);


#define SEG_1  0, 0, (LED_COUNT-1)	

void start_anim(uint8_t anim) {
	switch (anim) {
		case 0: // static 
			ws2812fx.setSegment(SEG_1, FX_MODE_STATIC, color_1, 0);
			break;
		case 1: // strobe white
			// ws2812fx.setSegment(SEG_1, FX_MODE_BLINK, WHITE, 200);
			ws2812fx.setSegment(SEG_1, FX_MODE_STATIC, WHITE, 0);
			is_strobe = 1;
			break;
		case 2:
			ws2812fx.setSegment(SEG_1, FX_MODE_COLOR_WIPE, color_1, 3000);
			break;
		case 3:
			ws2812fx.setSegment(SEG_1, FX_MODE_THEATER_CHASE, color_1, 50);
			break;
		case 4:
			ws2812fx.setSegment(SEG_1, FX_MODE_FIREWORKS);
			break;
		case 5:
			ws2812fx.setSegment(SEG_1, FX_MODE_RUNNING_LIGHTS);
			break;
		case 6:
			ws2812fx.setSegment(SEG_1, FX_MODE_TWINKLEFOX);
			break;
		case 7:
			ws2812fx.setSegment(SEG_1, FX_MODE_FIRE_FLICKER);
			break;
		case 8:
			ws2812fx.setSegment(SEG_1, FX_MODE_COMET);
			break;
		case 9:
			ws2812fx.setSegment(SEG_1, snakeMode2, color_1, 10);
			break;
		case 10:
			ws2812fx.setSegment(SEG_1, snakeMode, color_1, 10);
			break;
	}
}

int anim = 0;
uint8_t bright = 255;
uint8_t blackout = 0;

void DMX_task(void* parameter) {
	Serial.printf("Task DMX start\n");
	pinMode(DIP_PIN_0, INPUT_PULLUP);
	pinMode(DIP_PIN_1, INPUT_PULLUP);
	pinMode(DIP_PIN_2, INPUT_PULLUP);
	pinMode(DIP_PIN_3, INPUT_PULLUP);
	pinMode(DIP_PIN_4, INPUT_PULLUP);
	pinMode(DIP_PIN_5, INPUT_PULLUP);
	pinMode(DIP_PIN_6, INPUT_PULLUP);
	pinMode(DIP_PIN_7, INPUT_PULLUP);
	pinMode(DIP_PIN_8, INPUT_PULLUP);
	pinMode(DIP_PIN_9, INPUT_PULLUP);

	pinMode(15, OUTPUT);
	digitalWrite(15, 0);

	delay(10);

	Serial.printf("DMX start init\n");
	DMX::Initialize(input);
	#ifdef INVERT_RX
		uart_set_line_inverse(2, UART_SIGNAL_RXD_INV);
	#endif
	Serial.println("DMX initialized...");

	// uint16_t dip = (digitalRead(DIP_PIN_0) ? 0 : 1)
	// 	| (digitalRead(DIP_PIN_1) ? 0 : 1 << 1)
	// 	| (digitalRead(DIP_PIN_2) ? 0 : 1 << 2)
	// 	| (digitalRead(DIP_PIN_3) ? 0 : 1 << 3)
	// 	| (digitalRead(DIP_PIN_4) ? 0 : 1 << 4)
	// 	| (digitalRead(DIP_PIN_5) ? 0 : 1 << 5)
	// 	| (digitalRead(DIP_PIN_6) ? 0 : 1 << 6)
	// 	| (digitalRead(DIP_PIN_7) ? 0 : 1 << 7)
	// 	| (digitalRead(DIP_PIN_8) ? 0 : 1 << 8)
	// 	| (digitalRead(DIP_PIN_9) ? 0 : 1 << 9);
	// Serial.printf("Adress DMX: %d\n", dip);

	ws2812fx.setSegment(SEG_1, FX_MODE_COLOR_WIPE, color_1, 3000);
	uint8_t ctn = 0;
	uint8_t test_mode = 0;
	

	for (;;) { // infinite loop
		uint16_t dip = (digitalRead(DIP_PIN_0) ? 0 : 1)
			| (digitalRead(DIP_PIN_1) ? 0 : 1 << 1)
			| (digitalRead(DIP_PIN_2) ? 0 : 1 << 2)
			| (digitalRead(DIP_PIN_3) ? 0 : 1 << 3)
			| (digitalRead(DIP_PIN_4) ? 0 : 1 << 4)
			| (digitalRead(DIP_PIN_5) ? 0 : 1 << 5)
			| (digitalRead(DIP_PIN_6) ? 0 : 1 << 6)
			| (digitalRead(DIP_PIN_7) ? 0 : 1 << 7)
			| (digitalRead(DIP_PIN_8) ? 0 : 1 << 8);
		// Serial.printf("%d\n", dip);
		uint16_t dmx_adress = dip - 1;
		uint8_t  lite_mode = (digitalRead(DIP_PIN_9) ? 0 : 1);

		if (dip != 0) {
			test_mode = 0;
			if (DMX::IsHealthy() && dmx_adress <= 495) {
				digitalWrite(LED_STATUS_PIN, HIGH);
				if (dip != 0) {
					uint8_t print_info = 0;
					unsigned long now = millis();
					if (now - lastUpdate > 50) {
						print_info = 1;
						lastUpdate = now;
					}
					ctn = 0;
					int new_anim = anim;

					if (lite_mode) {
						// Serial.printf("LITE\n");
						new_anim = DMX::Read(dmx_adress + DMX_CHANNEL_LITE_ANIM) / 25;
						if (new_anim > 10)
							new_anim = 0;
					} else {
						if (DMX::Read(dmx_adress + DMX_CHANNEL_STROBE) > 127) // strobo white
							new_anim = 1;
						else if (DMX::Read(dmx_adress + DMX_CHANNEL_MODE_1) > 127)
							new_anim = 2;
						else if (DMX::Read(dmx_adress + DMX_CHANNEL_MODE_2) > 127)
							new_anim = 3;
						else if (DMX::Read(dmx_adress + DMX_CHANNEL_MODE_3) > 127)
							new_anim = 4;
						else if (DMX::Read(dmx_adress + DMX_CHANNEL_MODE_4) > 127)
							new_anim = 5;
						else if (DMX::Read(dmx_adress + DMX_CHANNEL_MODE_5) > 127)
							new_anim = 6;
						else if (DMX::Read(dmx_adress + DMX_CHANNEL_MODE_6) > 127)
							new_anim = 7;
						else if (DMX::Read(dmx_adress + DMX_CHANNEL_MODE_7) > 127)
							new_anim = 8;
						else if (DMX::Read(dmx_adress + DMX_CHANNEL_MODE_8) > 127)
							new_anim = 9;
						else if (DMX::Read(dmx_adress + DMX_CHANNEL_MODE_9) > 127)
							new_anim = 10;
						else
							new_anim = 0;
					}


					if (new_anim != anim) {
						Serial.printf("New anim %d\n", new_anim);
						anim = new_anim;
						ws2812fx.resetSegments();
						ws2812fx.strip_off();
						start_anim(anim);
						ws2812fx.setAllSpeed(speed);
					}

					if (DMX::Read(dmx_adress + DMX_CHANNEL_STROBE_G) > 127 || anim == 1) { // strobo general + strob white
						is_strobe = 1;
					}
					else {
						blackout = 0;
						is_strobe = 0;
					}

					if (print_info)
						Serial.printf("RGB: %03d, %03d, %03d", DMX::Read(dmx_adress + DMX_CHANNEL_COLOR_1_R), DMX::Read(dmx_adress + DMX_CHANNEL_COLOR_1_G), DMX::Read(dmx_adress + DMX_CHANNEL_COLOR_1_B));
					if (anim != 1) { // not strobe white
						uint32_t new_color = ((uint32_t)DMX::Read(dmx_adress + DMX_CHANNEL_COLOR_1_R) << 16) | ((uint32_t)DMX::Read(dmx_adress + DMX_CHANNEL_COLOR_1_G) << 8) | ((uint32_t)DMX::Read(dmx_adress + DMX_CHANNEL_COLOR_1_B));
						if (new_color != color_1) {
							color_1 = new_color;
							// if (new_anim == 0) // strobe color hack
							// 	ws2812fx.setSegment(SEG_1, FX_MODE_BLINK, color_1, 200);
						}
						ws2812fx.setAllColor(color_1);
					}

					if (print_info)
						Serial.printf(", dim: %03d", DMX::Read(dmx_adress + DMX_CHANNEL_BRIGHT));
					if (bright != DMX::Read(dmx_adress + DMX_CHANNEL_BRIGHT)) {
						bright = DMX::Read(dmx_adress + DMX_CHANNEL_BRIGHT);
					}

					if (anim == 0 || anim == 1) { // static or strob
						speed = 0;
						ws2812fx.setAllSpeed(speed);
					} else {
						int new_speed = 2650 - DMX::Read(dmx_adress + DMX_CHANNEL_SPEED) * 10;
						if (speed != new_speed) {
							speed = new_speed;
							ws2812fx.setAllSpeed(speed);
						}
					}

					int new_speed = 255 - DMX::Read(dmx_adress + DMX_CHANNEL_STROBE_SPEED);
					new_speed = map(new_speed, 0, 255, 200, 1000);
					if (strobe_speed != new_speed) {
						strobe_speed = new_speed;
						strob_ctn = 0;
					}

					if (print_info) {
						// Serial.printf(", speed: %04d", speed);
						Serial.printf(", anim: %02d", anim);
						Serial.printf(", adress: %03d", dmx_adress+1);
						Serial.printf(", strobe_speed: %d", strobe_speed);
						Serial.printf(", speed: %d", speed);
						Serial.printf(", is_strobe: %d", is_strobe);
						Serial.printf("\n");
					}
				}
			}
			else {
				if (ctn++ > 10)
					digitalWrite(LED_STATUS_PIN, LOW);
			}
		}
		else { // test mode
			if (test_mode == 0) {
				Serial.printf("test Mode\n");
				ws2812fx.resetSegments();
				ws2812fx.strip_off();
				bright = 50;
				blackout = 0;
				// chase_rainbow();
				ws2812fx.setSegment(SEG_1, FX_MODE_RAINBOW_CYCLE, color_1, 10);
			}
			test_mode = 1;
		}

		vTaskDelay(25 / portTICK_PERIOD_MS);
	}
}

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


void led_task(void* parameter) {
	Serial.printf("Task LED start\n");
	ws2812fx_p.init();

	ws2812fx.init();
	ws2812fx.setBrightness(255);
	ws2812fx.start();

	pinMode(LED_STATUS_PIN, OUTPUT);
	digitalWrite(LED_STATUS_PIN, led_blink);
	ws2812fx.setCustomShow(myCustomShow); // set the custom show function to forgo the NeoPixel
	for (;;) {
		ws2812fx.service();
		if (is_strobe) {
			strob_ctn++;
			// Serial.printf("stob %d > %d %d\n", strob_ctn, ((strobe_speed) / 100), blackout);
			if (strob_ctn > ((strobe_speed) / 100)) {
				blackout = !blackout;
				strob_ctn = 0;
			}
		}
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}

void setup() {
	Serial.begin(115200);
	Serial.printf("Start\n");

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
	vTaskDelay(1 / portTICK_PERIOD_MS);
}	