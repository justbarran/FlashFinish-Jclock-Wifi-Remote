#if !defined(ESP8266)
  #error This code is designed to run on ESP8266 and ESP8266-based boards! Please check your Tools->Board setting.
#endif

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ClickButton.h"
#include "ESPRotary.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

//#define TIMER_INTERRUPT_DEBUG         1

//#define SERIAL_DEBUG         1

//#define _TIMERINTERRUPT_LOGLEVEL_     1
// Select a Timer Clock
#define USING_TIM_DIV1                false           // for shortest and most accurate timer
#define USING_TIM_DIV16               false           // for medium time and medium accurate timer
#define USING_TIM_DIV256              true            // for longest timer but least accurate. Default

#include "ESP8266TimerInterrupt.h"
//https://miliohm.com/complete-tutorial-for-i2c-oled-0-96-128x64-arduino-display/
//https://randomnerdtutorials.com/esp8266-0-96-inch-oled-display-with-arduino-ide/


#define TIMER_INTERVAL_MS       1000

#define DISPLAY_STATE_IDLE 0
#define DISPLAY_STATE_RESET_TIME 1 
#define DISPLAY_STATE_SET_TIME 2
#define DISPLAY_STATE_SEND_TIME 3
#define DISPLAY_STATE_SET_BRIGHTNESS 4
#define DISPLAY_STATE_SEND_BRIGHTNESS 5
#define DISPLAY_STATE_INCREMENT_TIME 6
#define DISPLAY_STATE_PAUSE_TIME 7
#define DISPLAY_STATE_DECREMENT_TIME 8

#define DISPLAY_UPDATE_CLOCK_SET_TIME_X 0
#define DISPLAY_UPDATE_CLOCK_SET_TIME_Y 47
#define DISPLAY_UPDATE_CLOCK_SET_X 25
#define DISPLAY_UPDATE_CLOCK_SET_Y 0
#define DISPLAY_UPDATE_CLOCK_TIME_Y 20
#define DISPLAY_UPDATE_CLOCK_TIME_HRS_X 0
#define DISPLAY_UPDATE_CLOCK_TIME_HRS_MINS_X 32
#define DISPLAY_UPDATE_CLOCK_TIME_MINS_X 46
#define DISPLAY_UPDATE_CLOCK_TIME_MINS_SECS_X 77
#define DISPLAY_UPDATE_CLOCK_TIME_SECS_X 90


#define DISPLAY_UPDATE_BRIGHT_SET_X 0
#define DISPLAY_UPDATE_BRIGHT_SET_PERC_X 80
#define DISPLAY_UPDATE_BRIGHT_SET_Y DISPLAY_UPDATE_CLOCK_SET_TIME_Y


#define DIGIT_STATE_ZERO 0
#define DIGIT_STATE_HRS 1
#define DIGIT_STATE_MINS 2 
#define DIGIT_STATE_SECS 3

#define CLOCK_STATE_ZERO 0
#define CLOCK_STATE_COUNTUP 1
#define CLOCK_STATE_PAUSE 2 
#define CLOCK_STATE_COUNTDOWN 3
#define CLOCK_MAX 59
#define CLOCK_HRS_MAX 23
#define CLOCK_TIME_SECS_OFFSET 1


#define OLED_CLK_PIN        D1  //often used as SCL (I2C)
#define OLED_DATA_PIN       D2  //often used as SDA (I2C)
#define MAIN_BUTTON_PIN     D3  //connected to FLASH button, boot fails if pulled LOW
#define NA_PIN_1            D4  //connected to on-board LED, boot fails if pulled LOW
#define ENCODER_BUTTON_PIN  D5
#define ENCODER_DT_PIN      D6
#define ENCODER_CLK_PIN     D7
#define NA_PIN_2            D8  //Boot fails if pulled HIGH

#define BATTERY_VOLTAGE_ADC   A0
#define BATTERY_VOLTAGE_RANGE_MAX_mV   5000
#define BATTERY_VOLTAGE_HIGH_mV   4200
#define BATTERY_VOLTAGE_LOW_mV   2800

#define BATTERY_VOLTAGE_HIGH_ADC   (859)
#define BATTERY_VOLTAGE_LOW_ADC   (572)

// this number depends on your rotary encoder
#define CLICKS_PER_STEP 4   

#define BUTTON_DEBOUNCE_TIME 20 
#define BUTTON_MULTI_CLICK_TIME 300
#define BUTTON_LONG_CLICK_TIME 1000

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   128
#define LOGO_WIDTH    64
static const unsigned char PROGMEM logo_bmp[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x18, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0xcc, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xcc, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0xcc, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0xc8, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xc0, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf1, 0xc0, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x0f, 0xe3, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7f, 0xe3, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc3, 0xe0, 0x07, 0xf1, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x87, 0x00, 0x47, 0xe1, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1c, 0x00, 0x4f, 0xe0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x38, 0xb8, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x70, 0xa0, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xe4, 0x40, 0x3f, 0x80, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0xc3, 0x00, 0x3f, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x82, 0x1a, 0x7f, 0x03, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x10, 0x34, 0x7e, 0x1f, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x08, 0x74, 0xfe, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xe6, 0x20, 0xe8, 0xff, 0xfe, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x01, 0xd9, 0xff, 0xfc, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0x03, 0xb1, 0xfc, 0xd8, 0xe1, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xe0, 0x73, 0xf3, 0xb1, 0xd1, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x8c, 0x3c, 0x03, 0xee, 0xe3, 0xb1, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x88, 0x07, 0x80, 0x3d, 0xc7, 0x71, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x88, 0x00, 0xf0, 0xf3, 0x8e, 0xe1, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x98, 0x0c, 0x1f, 0xe7, 0x1d, 0xc1, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x9b, 0xdb, 0x03, 0xce, 0x3b, 0xb0, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x98, 0x16, 0x31, 0x9c, 0x77, 0x30, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x98, 0x0e, 0x7c, 0x38, 0xee, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x88, 0x1c, 0xff, 0xf1, 0xdc, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x8c, 0x38, 0xff, 0xe3, 0xb8, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x8c, 0x71, 0xff, 0xc7, 0x70, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0xcc, 0x83, 0xe7, 0xce, 0xe0, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xc4, 0x06, 0x0f, 0x9d, 0xc2, 0x31, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x10, 0x0f, 0x3b, 0x83, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x40, 0x1f, 0x37, 0x02, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x00, 0x1e, 0x6e, 0x10, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x00, 0x3c, 0xdc, 0x18, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x80, 0x3d, 0xd8, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0xc0, 0x78, 0x00, 0x40, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x60, 0x70, 0x01, 0x21, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x30, 0xf0, 0x13, 0x0a, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x18, 0xe0, 0x92, 0x04, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x09, 0xc0, 0x80, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0x80, 0x00, 0x50, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc3, 0x94, 0x06, 0xc1, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe3, 0x05, 0x36, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x05, 0x20, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xfc, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define WIFI_X   0
#define WIFI_Y    0
#define WIFI_HEIGHT   16
#define WIFI_WIDTH    20

#define WIFI_RSSI_ZERO  -90
#define WIFI_RSSI_LOW   -80
#define WIFI_RSSI_MID   -70
#define WIFI_RSSI_FULL  -60

#define BATTERY_HEIGHT   16
#define BATTERY_WIDTH    16
#define BATTERY_PERC_X   80
#define BATTERY_PERC_Y   2
#define BATTERY_LOGO_X   114
#define BATTERY_LOGO_Y   0

#define CLOCK_BATTERY_HEIGHT   16
#define CLOCK_BATTERY_WIDTH    16
#define CLOCK_BATTERY_PERC_X   DISPLAY_UPDATE_CLOCK_SET_X
#define CLOCK_BATTERY_PERC_Y   BATTERY_PERC_Y
#define CLOCK_BATTERY_LOGO_X   DISPLAY_UPDATE_CLOCK_SET_X + (12*3)
#define CLOCK_BATTERY_LOGO_Y   0


const unsigned char wifi_full_bmp [] PROGMEM = {
	0x00, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x0f, 0xff, 0x00, 0x3c, 0x03, 0xc0, 0x70, 0x00, 0xe0, 0xe3, 
	0xfc, 0x70, 0x47, 0xfe, 0x20, 0x1e, 0x07, 0x80, 0x18, 0x01, 0x80, 0x01, 0xf8, 0x00, 0x03, 0xfc, 
	0x00, 0x03, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00
};

const unsigned char wifi_mid_bmp [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 
	0xfc, 0x00, 0x07, 0xfe, 0x00, 0x1e, 0x07, 0x80, 0x18, 0x01, 0x80, 0x01, 0xf8, 0x00, 0x03, 0xfc, 
	0x00, 0x03, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00
};

const unsigned char wifi_low_bmp [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0x03, 0xfc, 
	0x00, 0x03, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00
};

const unsigned char wifi_zero_bmp [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00
};

const unsigned char battery_bmp [] PROGMEM = {
	// 'R, 16x16px
	0x03, 0xc0, 0x07, 0xe0, 0x1f, 0xf8, 0x1f, 0xf8, 0x10, 0x08, 0x10, 0x08, 0x17, 0xe8, 0x17, 0xe8, 
	0x10, 0x08, 0x17, 0xe8, 0x17, 0xe8, 0x10, 0x08, 0x17, 0xe8, 0x17, 0xe8, 0x1f, 0xf8, 0x1f, 0xf8
};

const unsigned char charge_bmp [] PROGMEM = {
	// 'R, 16x16px
	0x07, 0xc0, 0x07, 0xc0, 0x0f, 0x80, 0x0f, 0x80, 0x0f, 0xf8, 0x0f, 0xf0, 0x1f, 0xe0, 0x1f, 0xe0, 
	0x01, 0xc0, 0x01, 0xc0, 0x03, 0x80, 0x03, 0x80, 0x03, 0x00, 0x02, 0x00, 0x02, 0x00, 0x04, 0x00
};

const unsigned char clock_bmp [] PROGMEM = {
	0x18, 0x60, 0x78, 0x78, 0x6f, 0xdc, 0x50, 0x28, 0x21, 0x10, 0x41, 0x08, 0x81, 0x04, 0x81, 0x04, 
	0xbf, 0x04, 0x80, 0x04, 0x80, 0x04, 0xc0, 0x04, 0x40, 0x08, 0x20, 0x10, 0x18, 0x20, 0x0f, 0xc0
};

ClickButton encoderButton(ENCODER_BUTTON_PIN, LOW, CLICKBTN_PULLUP);
ClickButton mainButton(MAIN_BUTTON_PIN, LOW, CLICKBTN_PULLUP);
ESPRotary encoder;
// Init ESP8266 timer 1
ESP8266Timer ITimer;

#define UPDATE_BATTERY_TIME_MS    10000
uint32_t updateBatteryTime = 0;
uint32_t updateBatteryTimeLast = 0;
uint8_t updateBatteryChargeFlag = false;

volatile uint8_t displayState = DISPLAY_STATE_IDLE;
uint8_t digitState = DISPLAY_STATE_IDLE;

volatile uint8_t timeValueHrs = 0;
volatile uint8_t timeValueMins = 0;
volatile uint8_t timeValueSecs = 0;
volatile uint8_t timeState = CLOCK_STATE_ZERO;
volatile uint8_t timeUpdateFlag= false;
volatile uint8_t clockUpdateFlag= false;
volatile uint32_t lastMillis = 0;


uint8_t timeValueHrsLast = 1;
uint8_t timeValueMinsLast = 1;
uint8_t timeValueSecsLast = 1;


uint8_t brightnessValue = 5;
uint8_t brightnessPerctLast = 0;

uint16_t adcValue= 0;

uint8_t internalBatteryPercent = 0;
uint8_t internalBatteryPercentLast = 0;

uint8_t clockBatteryPercent = 0;
uint8_t clockBatteryPercentLast = 0;

//=======================================================================
void IRAM_ATTR TimerHandler()
{
  if(CLOCK_STATE_COUNTUP==timeState)
  {
    timeValueSecs++;
    if(timeValueSecs>59)
    {
      timeValueSecs = 0;
      timeValueMins++;
      if(timeValueMins>59)
      {
        timeValueMins = 0;
        timeValueHrs++;
        if(timeValueHrs>CLOCK_HRS_MAX)
        {
          timeValueHrs = 0;
        }
      }
    }
    timeUpdateFlag= true;
  }
  else if (CLOCK_STATE_COUNTDOWN==timeState)
  {
    if(timeValueSecs==0)
    {      
      if(timeValueMins>0)
      {
        timeValueSecs=60;
        timeValueMins--;
      }
      else if(timeValueHrs>0 )
      {
        timeValueHrs--;        
        timeValueMins=59;
        timeValueSecs=59;     
      }
      else
      {
        timeState = CLOCK_STATE_ZERO;
      }      
    }
    else
    {
      timeValueSecs--;
    }
    timeUpdateFlag=true;
  }
  clockUpdateFlag= true;
#if (TIMER_INTERRUPT_DEBUG > 0)
  Serial.println("Delta ms = " + String(millis() - lastMillis));
  lastMillis = millis();
#endif
}

const char* ssid = "ITSClock_0005";
const char* password = "";
//Your Domain name with URL path or IP address with path
/* http://192.168.1.253/action_page.php?SetTimeUpOver=&SetTimeDwnStp=&SetTimeDwnbUp=&PauseClock=&AutoBright=&SetBrht=&AutoBattery=&ShowBattery=&DiagnosticCommand= */
/* 
params = {
    'SetTimeUpOver': '000000',
    'SetTimeDwnStp': '000000',
    'SetTimeDwnbUp': '000000',
    'PauseClock': 'enable/disable',
    'AutoBright': 'off/on',
    'SetBrht': '0/100',
    'AutoBattery': '',
    'ShowBattery': '',
    'DiagnosticCommand': ''
}

*/
#define CLOCK_PARM_TIME_UP              "SetTimeUpOver"
#define CLOCK_PARM_TIME_DOWN            "SetTimeDwnStp"
#define CLOCK_PARM_TIME_DOWN_UP         "SetTimeDwnbUp"
#define CLOCK_PARM_PAUSE_CLOCK          "PauseClock"
#define CLOCK_PARM_AUTO_BRIGHT          "AutoBright"
#define CLOCK_PARM_SET_BRIGHTNESS       "SetBrht"
#define CLOCK_PARM_AUTO_BATTERY         "AutoBattery"
#define CLOCK_PARM_SHOW_BATTERY         "ShowBattery"
#define CLOCK_PARM_DIAG                 "DiagnosticCommand"

#define CLOCK_PAUSE_ENABLE "enable"
#define CLOCK_PAUSE_DISABLE "disable"
#define CLOCK_AUTO_BRIGHTNESS_ON "on"
#define CLOCK_AUTO_BRIGHTNESS_OFF "off"
#define CLOCK_SHOW_BATTERY_ON "show"

#define CLOCK_SEND_DELAY 250

// Define base URL and parameters
const char* baseURL = "http://192.168.1.253/action_page.php?";
const char* parameterNames[] = {CLOCK_PARM_TIME_UP, CLOCK_PARM_TIME_DOWN,CLOCK_PARM_TIME_DOWN_UP,CLOCK_PARM_PAUSE_CLOCK,CLOCK_PARM_AUTO_BRIGHT, CLOCK_PARM_SET_BRIGHTNESS, CLOCK_PARM_AUTO_BATTERY,CLOCK_PARM_SHOW_BATTERY,CLOCK_PARM_DIAG};
char parameterValues[9][10]; // Array of char arrays to store values, assuming max length of 50 chars per value

char timeString[7]; // 6 characters + 1 for null terminator
char brightnessString[4]; // 6 characters + 1 for null terminator

WiFiClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(50);
  Serial.println("\n\n Flash Finish Tech");
  delay(100);
  // Interval in microsecs
  if (ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 1000, TimerHandler))
  {
    lastMillis = millis();
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(lastMillis);
  }
  else
    Serial.println(F("Can't set ITimer correctly. Select another freq. or interval"));

  //pinMode(MAIN_BUTTON_PIN, INPUT); // set pin to input
  //pinMode(ENCODER_BUTTON_PIN, INPUT); // set pin to input
  pinMode(ENCODER_DT_PIN, INPUT); // set pin to input
  pinMode(ENCODER_CLK_PIN, INPUT); // set pin to input
   // Setup button timers (all in milliseconds / ms)
  // (These are default if not set, but changeable for convenience)
  encoderButton.debounceTime   = BUTTON_DEBOUNCE_TIME;   // Debounce timer in ms
  encoderButton.multiclickTime = BUTTON_MULTI_CLICK_TIME;  // Time limit for multi clicks
  encoderButton.longClickTime  = BUTTON_LONG_CLICK_TIME; // time until "held-down clicks" register

  mainButton.debounceTime   = BUTTON_DEBOUNCE_TIME;   // Debounce timer in ms
  mainButton.multiclickTime = BUTTON_MULTI_CLICK_TIME;  // Time limit for multi clicks
  mainButton.longClickTime  = BUTTON_LONG_CLICK_TIME; // time until "held-down clicks" register

  encoder.begin(ENCODER_DT_PIN, ENCODER_CLK_PIN, CLICKS_PER_STEP,-1,CLOCK_MAX+1);
  encoder.setChangedHandler(rotate);
  //encoder.setLeftRotationHandler(showDirection);
  //encoder.setRightRotationHandler(showDirection);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.drawBitmap(0,0,logo_bmp, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);
  display.setTextSize(2);             // Normal 1:1 pixel scale 
  display.setCursor(0,0);             // Start at top-left corner
  display.print("FLASH TECH");
  display.setCursor(0,20);             // Start at top-left corner
  display.print("    X     ");
  display.setCursor(0,40);             // Start at top-left corner
  display.print("EXTRA MILE");
  display.display();
  delay(2000);

  display.clearDisplay();
  display.display();
  updateBattery(true);  
  updateClock(timeValueHrs,timeValueMins,timeValueSecs,displayState,digitState);
  updateBightness(brightnessValue,displayState, true);
  
  clearParameter();
  display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
  display.setTextSize(2);             // Normal 1:1 pixel scale 
  display.setCursor(0,0);             // Start at top-left corner  
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  updateWifi(true);

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP()); 
  Serial.println("\n\n Set Up Done");
}

/*==================================================================*/
/*==================================================================*/
/*==================================================================*/

void updateClock(uint8_t hrs, uint8_t mins, uint8_t secs, uint8_t displayState, uint8_t setState) 
{
  display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
  display.setTextSize(2);             // Normal 1:1 pixel scale 
  display.setCursor(DISPLAY_UPDATE_CLOCK_SET_X,DISPLAY_UPDATE_CLOCK_SET_Y);             // Start at top-left corner

  if(DISPLAY_STATE_SET_TIME==displayState)
  {    
    display.print("TIME ");
    display.fillRect(DISPLAY_UPDATE_CLOCK_TIME_HRS_X,DISPLAY_UPDATE_CLOCK_SET_TIME_Y,display.width(),display.height()-DISPLAY_UPDATE_CLOCK_SET_TIME_Y, SSD1306_BLACK);
    display.setTextSize(3);
    if(setState == DIGIT_STATE_HRS)
    {
      display.setCursor(DISPLAY_UPDATE_CLOCK_TIME_HRS_X,DISPLAY_UPDATE_CLOCK_SET_TIME_Y);             // Start at top-left corner
      display.print("^^");
    }
    else if(setState == DIGIT_STATE_MINS)
    {
      display.setCursor(DISPLAY_UPDATE_CLOCK_TIME_MINS_X,DISPLAY_UPDATE_CLOCK_SET_TIME_Y);             // Start at top-left corner
      display.print("^^");
    }
    else if(setState == DIGIT_STATE_SECS)
    {
      display.setCursor(DISPLAY_UPDATE_CLOCK_TIME_SECS_X,DISPLAY_UPDATE_CLOCK_SET_TIME_Y);             // Start at top-left corner
      display.print("^^");
    } 
    display.display();
  }
  else if(DISPLAY_STATE_SET_BRIGHTNESS==displayState)
  {    
    display.print("LIGHT");
  }
  else
  {
    //display.print("     ");
  }

  display.setTextSize(3);             // Normal 1:1 pixel scale    
  if(hrs>CLOCK_HRS_MAX)hrs=23;
  if(mins>CLOCK_MAX)mins=59;
  if(secs>CLOCK_MAX)secs=59;

  if(timeValueHrsLast!=hrs)
  {
    display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
    display.setCursor(DISPLAY_UPDATE_CLOCK_TIME_HRS_X,DISPLAY_UPDATE_CLOCK_TIME_Y);             // Start at top-left corner
    if(hrs<10)display.print(F("0"));
    display.print(hrs); 

    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(DISPLAY_UPDATE_CLOCK_TIME_HRS_MINS_X,DISPLAY_UPDATE_CLOCK_TIME_Y);             // Start at top-left corner      
    display.print(":");
    timeValueHrsLast=hrs;
  }

  if(timeValueMinsLast!=mins)
  {
    display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
    display.setCursor(DISPLAY_UPDATE_CLOCK_TIME_MINS_X,DISPLAY_UPDATE_CLOCK_TIME_Y);             // Start at top-left corner
    if(mins<10)display.print(F("0"));
    display.print(mins);  
  
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(DISPLAY_UPDATE_CLOCK_TIME_MINS_SECS_X,DISPLAY_UPDATE_CLOCK_TIME_Y);             // Start at top-left corner       
    display.print(":");
    timeValueMinsLast=mins;
  }

  if(timeValueSecsLast!=secs)
  {
    display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
    display.setCursor(DISPLAY_UPDATE_CLOCK_TIME_SECS_X,DISPLAY_UPDATE_CLOCK_TIME_Y);             // Start at top-left corner
    if(secs<10)display.print(F("0"));
    display.print(secs);
    timeValueSecsLast=secs;
  }  
  display.display();
}


/*==================================================================*/
/*==================================================================*/
/*==================================================================*/



void updateBightness(uint8_t brightness, uint8_t displayState, uint8_t force) 
{
  display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
  display.setTextSize(2);             // Normal 1:1 pixel scale 
  display.setCursor(DISPLAY_UPDATE_CLOCK_SET_X,DISPLAY_UPDATE_CLOCK_SET_Y);             // Start at top-left corner
  
  if(displayState==DISPLAY_STATE_SET_TIME)
  {    
    display.print("TIME");
  }
  else if(displayState==DISPLAY_STATE_SET_BRIGHTNESS)
  {    
    display.print("LIGHT");
  }
  else
  {
    display.print("     ");    
  }  

  uint8_t brightnessPerct = brightness * 10;
  if(brightnessPerct>100)brightnessPerct=100;
  
  if(true == force || brightnessPerct!=brightnessPerctLast )
  {
    display.fillRect(DISPLAY_UPDATE_CLOCK_TIME_HRS_X,DISPLAY_UPDATE_CLOCK_SET_TIME_Y,display.width(),display.height()-DISPLAY_UPDATE_CLOCK_SET_TIME_Y, SSD1306_BLACK);
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
    display.setCursor(DISPLAY_UPDATE_BRIGHT_SET_X,DISPLAY_UPDATE_BRIGHT_SET_Y); 
    display.print("DISP:     ");
    display.setCursor(DISPLAY_UPDATE_BRIGHT_SET_PERC_X,DISPLAY_UPDATE_BRIGHT_SET_Y);             // Start at top-left corner
    if(brightnessPerct>20)
    {            
      if(brightnessPerct<100)display.print(F(" "));
      if(brightnessPerct<10)display.print(F(" "));
      display.print(brightnessPerct);
      display.print(F("%"));
      display.display();
    }
    else if(brightnessPerct> 0)
    {
      display.print(" OFF");
    }
    else
    {
      display.print("AUTO");
    }
    brightnessPerctLast = brightnessPerct;
    formatBrightness(brightnessPerct, brightnessString);
  }
  display.display(); 
}

/*==================================================================*/
/*==================================================================*/
/*==================================================================*/


void displaySent(bool force) 
{
    display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
    display.setTextSize(2);             // Normal 1:1 pixel scale 
    display.setCursor(DISPLAY_UPDATE_CLOCK_SET_X,DISPLAY_UPDATE_CLOCK_SET_Y);             // Start at top-left corner
    display.print(" ^^^ ");
    display.display(); 
}

void displaySentClear(bool force) 
{
    display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
    display.setTextSize(2);             // Normal 1:1 pixel scale 
    display.setCursor(DISPLAY_UPDATE_CLOCK_SET_X,DISPLAY_UPDATE_CLOCK_SET_Y);             // Start at top-left corner
    display.print("     ");
    display.display(); 
}

/*==================================================================*/
/*==================================================================*/
/*==================================================================*/

void updateWifi(bool force) 
{
  if(WiFi.status() == WL_CONNECTED)
  {
    int8_t rssiValue = WiFi.RSSI();
    display.setCursor(0,0);             // Start at top-left corner  
    display.fillRect(0,0,WIFI_WIDTH,WIFI_HEIGHT, SSD1306_BLACK);
    if(rssiValue > WIFI_RSSI_FULL)
    {
      display.drawBitmap(0,0,wifi_full_bmp, WIFI_WIDTH, WIFI_HEIGHT, 1);
    }
    else if(rssiValue > WIFI_RSSI_MID )
    {
      display.drawBitmap(0,0,wifi_mid_bmp, WIFI_WIDTH, WIFI_HEIGHT, 1);
    }
    else if (rssiValue > WIFI_RSSI_LOW )
    {
      display.drawBitmap(0,0,wifi_low_bmp, WIFI_WIDTH, WIFI_HEIGHT, 1);
    }
    else
    {
      display.drawBitmap(0,0,wifi_zero_bmp, WIFI_WIDTH, WIFI_HEIGHT, 1);
    }
    display.display();
  }
  else 
  {
    clockBatteryPercent = 0;
    display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
    display.setTextSize(2);             // Normal 1:1 pixel scale 
    display.setCursor(0,0);             // Start at top-left corner  
    display.print("       "); 
    updateBattery(true);   
    display.setCursor(0,0);             // Start at top-left corner  
    WiFi.begin(ssid, password);
    Serial.println("Connecting");
    uint8_t dashcount = 0;
    while(WiFi.status() != WL_CONNECTED) {    
      Serial.print(".");
      display.print(">"); 
      dashcount++;               
      if(dashcount>3)  
      {
        dashcount = 0;
        updateBattery(false); 
        display.setCursor(0,0);             // Start at top-left corner  
        display.print("     "); 
        display.setCursor(0,0);             // Start at top-left corner  
      }       
      display.display();    
      delay(500);
    }
    display.setCursor(0,0);             // Start at top-left corner  
    display.print("     "); 
    display.display();    
  }     
}

/*==================================================================*/
/*==================================================================*/
/*==================================================================*/

void updateBattery(uint8_t force) 
{
  updateBatteryTime =millis();
  if(((updateBatteryTime-updateBatteryTimeLast)>UPDATE_BATTERY_TIME_MS)||true==force)
  {
    adcValue = analogRead(A0);  
    if(adcValue>(BATTERY_VOLTAGE_HIGH_ADC+10))
    {
      updateBatteryChargeFlag = true;
    }
    else
    {
      updateBatteryChargeFlag = false;
    }

    if(adcValue>BATTERY_VOLTAGE_HIGH_ADC)adcValue=BATTERY_VOLTAGE_HIGH_ADC;
    else if(adcValue<BATTERY_VOLTAGE_LOW_ADC) adcValue=BATTERY_VOLTAGE_LOW_ADC;    
    internalBatteryPercent = (uint8_t)map((long)adcValue, (long)BATTERY_VOLTAGE_LOW_ADC, (long)BATTERY_VOLTAGE_HIGH_ADC, (long)0, (long)100);

    if(internalBatteryPercentLast!=internalBatteryPercent)
    {
      if (internalBatteryPercent > internalBatteryPercentLast)
      {
         if((internalBatteryPercent-internalBatteryPercentLast)> 1)
         {
           force = true;
         }
      }
      if (internalBatteryPercent < internalBatteryPercentLast)
      {
        if((internalBatteryPercentLast-internalBatteryPercent) > 1)
         {
           force = true;
         }
      }
    }
    
    if(DISPLAY_STATE_IDLE==displayState)
    {
      updateParameter(CLOCK_PARM_SHOW_BATTERY,CLOCK_SHOW_BATTERY_ON);
      if(clockBatteryPercent!=clockBatteryPercentLast)
      {
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text      
        if(clockBatteryPercent>0)
        {
          display.setCursor(CLOCK_BATTERY_PERC_X,CLOCK_BATTERY_PERC_Y);             // Start at top-left corner
          if(clockBatteryPercent<100)display.print(F(" "));
          if(clockBatteryPercent<10)display.print(F(" "));
          display.print(clockBatteryPercent);
          display.print(" ");        
          display.drawBitmap(CLOCK_BATTERY_LOGO_X,CLOCK_BATTERY_LOGO_Y,clock_bmp, CLOCK_BATTERY_WIDTH, CLOCK_BATTERY_HEIGHT, 1);
        }
        else
        {
          display.print("    ");
        }
      }
    }

    if(true==force)
    {
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE,SSD1306_BLACK);        // Draw white text
      
      display.setCursor(BATTERY_PERC_X,BATTERY_PERC_Y);             // Start at top-left corner
      if(updateBatteryChargeFlag==false)
      {
        if(internalBatteryPercent<100)display.print(F(" "));
        if(internalBatteryPercent<10)display.print(F(" "));
        display.print(internalBatteryPercent);
        display.print(" ");        
        display.drawBitmap(BATTERY_LOGO_X,BATTERY_LOGO_Y,battery_bmp, BATTERY_WIDTH, BATTERY_HEIGHT, 1);
      }
      else
      {
        display.print("    ");
        display.drawBitmap(BATTERY_LOGO_X,BATTERY_LOGO_Y,charge_bmp, BATTERY_WIDTH, BATTERY_HEIGHT, 1);
      }       
      display.display();
      internalBatteryPercentLast = internalBatteryPercent;
    }
    updateBatteryTimeLast = updateBatteryTime;
  }
}

// on change
void rotate(ESPRotary& r) {
    int temp = r.getPosition();
    if(DISPLAY_STATE_SET_TIME == displayState)
    {
      if(digitState == DIGIT_STATE_HRS)
      {
        if(temp>CLOCK_HRS_MAX)
        {
          temp = 0;
          encoder.resetPosition(temp);
        }
        else if(temp == -1)
        {
          temp = CLOCK_HRS_MAX;
          encoder.resetPosition(temp);
        }
      }
      {
        if(temp>CLOCK_MAX)
        {
          temp = 0;
          encoder.resetPosition(temp);
        }
        else if(temp == -1)
        {
          temp = CLOCK_MAX;
          encoder.resetPosition(temp);
        }
      }      

      if(digitState == DIGIT_STATE_HRS)
      {
        timeValueHrs=(uint8_t)temp;
      }
      else if(digitState == DIGIT_STATE_MINS)
      {
        timeValueMins=(uint8_t)temp;
      }
      else if(digitState == DIGIT_STATE_SECS)
      {
        timeValueSecs=(uint8_t)temp;
      }
      else if(digitState>DIGIT_STATE_SECS)
      {
        digitState = DIGIT_STATE_ZERO;
      }
      updateClock(timeValueHrs,timeValueMins,timeValueSecs,displayState,digitState);
    }
    else if(DISPLAY_STATE_SET_BRIGHTNESS == displayState) 
    {
      if(temp > 10)
      {
        temp = 10;
        encoder.resetPosition(temp);
      }
      else if(temp < 0)
      {
        temp = 0;
        encoder.resetPosition(temp);
      }
      brightnessValue = (uint8_t)temp;
      updateBightness(brightnessValue,displayState, true);
    }    
}

void formatTime(uint8_t hrs, uint8_t mins, uint8_t secs, char* outBuffer) {
    sprintf(outBuffer, "%02u%02u%02u", hrs, mins, secs);
}

void formatBrightness(uint8_t brightness, char* outBuffer) {
    //brightness = brightness*10;
    sprintf(outBuffer, "%u", brightness); // Formats the unsigned integer as a simple number
}

void clearParameter() {
  // Initialize parameter values to empty
  for (int i = 0; i < 9; i++) {
    parameterValues[i][0] = '\0';
  }
}

unsigned int extractBatteryLevel(const String& payload) {
  int startIndex = payload.indexOf("Battery Level:") + 14; // 14 characters in "Battery Level:"
  if (startIndex > 13) { // Check if "Battery Level:" was found
    int endIndex = payload.indexOf('%', startIndex);
    if (endIndex > startIndex) {
      String batteryString = payload.substring(startIndex, endIndex);
      float batteryFloat = batteryString.toFloat(); // Convert to float first to handle decimal
      return static_cast<unsigned int>(batteryFloat); // Convert to unsigned int, discarding decimal part
    }
  }
  return 0; // Return 0 if not found or in case of an error
}

void updateParameter(const char* key, const char* value) {
  bool getbattery = false;
  for (int i = 0; i < sizeof(parameterNames) / sizeof(parameterNames[0]); i++) {
    if (strcmp(parameterNames[i], key) == 0) {
      strcpy(parameterValues[i], value);
      // Serial.print("Updated ");
      // Serial.print(key);
      // Serial.print(" to ");
      // Serial.println(value);
      if (strcmp(CLOCK_PARM_SHOW_BATTERY, key) == 0)
      {
        getbattery = true;
      }
      sendGetRequest(getbattery);
      clearParameter();
      return;
    }
  }
  //Serial.print("Parameter not found: ");
  //Serial.println(key);
}

void sendGetRequest(bool getbattery) {
  char url[300] = {0}; // Large enough to hold the base URL and all parameters
  strcpy(url, baseURL);
  for (int i = 0; i < sizeof(parameterNames) / sizeof(parameterNames[0]); i++) {
    if (i > 0) strcat(url, "&");
    strcat(url, parameterNames[i]);
    strcat(url, "=");
    strcat(url, parameterValues[i]);
  }
  //Serial.println(url);  
  
  if (WiFi.status() == WL_CONNECTED) {

    if(getbattery != true)displaySent(false);
    HTTPClient http;
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      if(getbattery==true)
      {
        String payload = http.getString();
        clockBatteryPercent = extractBatteryLevel(payload);
        //Serial.print("Received payload: ");
        //Serial.println(payload);
      }
      
    } else {
      //Serial.print("GET request failed: ");
      //Serial.println(http.errorToString(httpCode).c_str());
    }
    http.end();
    if(getbattery != true)displaySentClear(false);
    updateBatteryTimeLast = millis();
  } 
  else {
    //Serial.println("WiFi not connected");
  }  
  
}

void loop() 
{
  // put your main code here, to run repeatedly:
  // Update button state
  encoderButton.Update();
  mainButton.Update();
  encoder.loop();
  updateBattery(false); 

  if(true == timeUpdateFlag && DISPLAY_STATE_IDLE == displayState)
  {
    timeUpdateFlag = false;
    updateClock(timeValueHrs,timeValueMins,timeValueSecs,displayState,digitState);
  }

  if(encoderButton.clicks == 1) 
  {
    //Serial.println("encoder single click");
    if(DISPLAY_STATE_SET_TIME == displayState)
    {
      digitState++;
      updateClock(timeValueHrs,timeValueMins,timeValueSecs,displayState,digitState);
      if(digitState == DIGIT_STATE_HRS)
      {
        encoder.resetPosition(timeValueHrs);
        
      }
      else if(digitState == DIGIT_STATE_MINS)
      {
        encoder.resetPosition(timeValueMins);
      }
      else if(digitState == DIGIT_STATE_SECS)
      {
        encoder.resetPosition(timeValueSecs);
      }
      else if(digitState>DIGIT_STATE_SECS)
      {
        digitState = DIGIT_STATE_ZERO;
        displayState = DISPLAY_STATE_IDLE;
        updateBightness(brightnessValue,displayState, true);        
      }
    }
    else if(DISPLAY_STATE_SET_BRIGHTNESS == displayState)
    {
      digitState = DIGIT_STATE_ZERO;
      displayState = DISPLAY_STATE_SEND_BRIGHTNESS;
      updateBightness(brightnessValue,displayState, true);
    }
  }

  if(encoderButton.clicks == 2) 
  {
    //Serial.println("encoder double click");
    if(DISPLAY_STATE_IDLE==displayState)
    {
      displayState = DISPLAY_STATE_SET_TIME;
      digitState = DIGIT_STATE_ZERO;
      timeState = CLOCK_STATE_PAUSE;
      updateClock(timeValueHrs,timeValueMins,timeValueSecs,displayState,digitState);
    }
    else if(DISPLAY_STATE_SET_TIME==displayState)
    {
      digitState = DIGIT_STATE_ZERO;
      displayState = DISPLAY_STATE_IDLE;
      updateClock(timeValueHrs,timeValueMins,timeValueSecs,displayState,digitState);
      updateBightness(brightnessValue,displayState, true); 
    }
  }
  // blink even faster if triple clicked
  if(encoderButton.clicks == 3) 
  {
    //Serial.println("encoder triple click");
    if(DISPLAY_STATE_IDLE==displayState)
    {
      encoder.resetPosition(brightnessValue);
      displayState = DISPLAY_STATE_SET_BRIGHTNESS;
      updateBightness(brightnessValue,displayState, true); 
    }
  }

  if(encoderButton.clicks == -1) 
  {
    //Serial.println("encoder hold down");
    if(DISPLAY_STATE_IDLE==displayState)
    { 
      //Serial.println("main single click");
      timeState = CLOCK_STATE_PAUSE; 
      displayState = DISPLAY_STATE_SEND_TIME;
    } 
  }


  if(mainButton.clicks == 1) 
  {
    if(DISPLAY_STATE_IDLE==displayState)
    { 
      //Serial.println("main single click");
      timeState = CLOCK_STATE_COUNTUP; 
      displayState = DISPLAY_STATE_INCREMENT_TIME;
    } 
    else if(DISPLAY_STATE_SET_TIME==displayState)
    {
      digitState = DIGIT_STATE_ZERO;
      displayState = DISPLAY_STATE_IDLE;
      updateClock(timeValueHrs,timeValueMins,timeValueSecs,displayState,digitState);
      updateBightness(brightnessValue,displayState, true); 
    }
    else if(DISPLAY_STATE_SET_BRIGHTNESS == displayState)
    {
      digitState = DIGIT_STATE_ZERO;
      displayState = DISPLAY_STATE_SEND_BRIGHTNESS;
      updateBightness(brightnessValue,displayState, true);
    }
  }

  if(mainButton.clicks == 2) 
  {
    if(DISPLAY_STATE_IDLE==displayState)
    { 
      //Serial.println("main double click");
      timeState = CLOCK_STATE_PAUSE;
      displayState = DISPLAY_STATE_PAUSE_TIME;
    }
  }
  // blink even faster if triple clicked
  if(mainButton.clicks == 3) 
  {
    if(DISPLAY_STATE_IDLE==displayState)
    {  
      //Serial.println("main triple click");      
      timeState = CLOCK_STATE_COUNTDOWN; 
      displayState = DISPLAY_STATE_DECREMENT_TIME; 
      
    } 
  }

  if(mainButton.clicks == -1) 
  {
    //Serial.println("main hold down");
    if(DISPLAY_STATE_IDLE==displayState)
    {      
      timeState = CLOCK_STATE_ZERO; 
      timeValueHrs = 0;
      timeValueMins= 0;
      timeValueSecs= 0;
      updateClock(timeValueHrs,timeValueMins,timeValueSecs,displayState,digitState);
      displayState = DISPLAY_STATE_SEND_TIME;
    }
  }

  if(clockUpdateFlag == true)
  {      
    switch (displayState) {
      case DISPLAY_STATE_IDLE:
        // do something when var equals 1
        break;
      case DISPLAY_STATE_RESET_TIME:
        // do something when var equals 1
        break;
      case DISPLAY_STATE_SET_TIME:        
        break;
      case DISPLAY_STATE_SEND_TIME:
        if(timeState == CLOCK_STATE_ZERO)
        {
          formatTime(timeValueHrs,timeValueMins,timeValueSecs,timeString);
        }
        else
        {
          formatTime(timeValueHrs,timeValueMins,timeValueSecs+CLOCK_TIME_SECS_OFFSET,timeString);
        }        
        updateParameter(CLOCK_PARM_TIME_DOWN_UP,timeString);
        //updateParameter(CLOCK_PARM_TIME_UP,timeString);        
        delay(CLOCK_SEND_DELAY);
        updateParameter(CLOCK_PARM_PAUSE_CLOCK,CLOCK_PAUSE_ENABLE);
        displayState=DISPLAY_STATE_IDLE;
        break;
      case DISPLAY_STATE_SET_BRIGHTNESS:

        break;
      case DISPLAY_STATE_SEND_BRIGHTNESS:
        if(brightnessValue==0)
        {
          updateParameter(CLOCK_PARM_AUTO_BRIGHT,CLOCK_AUTO_BRIGHTNESS_ON);
        }
        else
        {
          updateParameter(CLOCK_PARM_SET_BRIGHTNESS,brightnessString);
          delay(CLOCK_SEND_DELAY);
          updateParameter(CLOCK_PARM_AUTO_BRIGHT,CLOCK_AUTO_BRIGHTNESS_OFF);
        }
        displayState=DISPLAY_STATE_IDLE;
        break;
      case DISPLAY_STATE_INCREMENT_TIME:
        if(timeValueSecs>0)
        {
          formatTime(timeValueHrs,timeValueMins,timeValueSecs-CLOCK_TIME_SECS_OFFSET,timeString);
        }
        else
        {
          formatTime(timeValueHrs,timeValueMins,timeValueSecs,timeString);
        }        
        updateParameter(CLOCK_PARM_TIME_UP,timeString);
        displayState=DISPLAY_STATE_IDLE;
        break;
      case DISPLAY_STATE_PAUSE_TIME:
        //updateParameter(CLOCK_PARM_TIME_DOWN_UP,timeString);
        //formatTime(hrs,mins,secs,timeString);
        updateParameter(CLOCK_PARM_PAUSE_CLOCK,CLOCK_PAUSE_ENABLE);
        displayState=DISPLAY_STATE_IDLE;
        break;
      case DISPLAY_STATE_DECREMENT_TIME:
        formatTime(timeValueHrs,timeValueMins,timeValueSecs+CLOCK_TIME_SECS_OFFSET,timeString);
        updateParameter(CLOCK_PARM_TIME_DOWN,timeString);
        displayState=DISPLAY_STATE_IDLE;
        break;
      
      default:
        // if nothing else matches, do the default
        // default is optional
        break;
        
      
    }
    clockUpdateFlag = false;
    updateWifi(false);  
  }
}
