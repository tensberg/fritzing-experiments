#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <Encoder.h>
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define PIN            6
#define BUTTON         10
#define ENCODER_A      8
#define ENCODER_B      9
#define I2C_CLOCK      13
#define I2C_DATA       11

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      60

#define BRIGHTNESS 24

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Encoder knob(8,9);

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ I2C_CLOCK, /* data=*/ I2C_DATA, /* reset=*/ U8X8_PIN_NONE);

int activePixel = 0;

int buttonOn = HIGH;

#define COLOR_COUNT 4
uint32_t colors[COLOR_COUNT] = { 
  pixels.Color(BRIGHTNESS, 0, 0),
  pixels.Color(0, BRIGHTNESS, 0),
  pixels.Color(0, 0, BRIGHTNESS),
  pixels.Color(BRIGHTNESS/3, BRIGHTNESS/3, BRIGHTNESS/3),
};

char *colorNames[] = {
  "Rot",
  "Gruen",
  "Blau",
  "Weiss"
};

int colorIndex = 0;

void setup() {
  pinMode(BUTTON, INPUT);
  
  pixels.begin(); // This initializes the NeoPixel library.
  u8g2.begin();
  
  paintActivePixel();
}

void loop() {

  int newButtonOn = digitalRead(BUTTON);

  if (newButtonOn != buttonOn) {
    if (newButtonOn == LOW) {
      colorIndex = (colorIndex + 1) % COLOR_COUNT;
      paintActivePixel();
    }
    buttonOn = newButtonOn;
  }
  
  long knobPosition = knob.read();
  int newActivePixel = knobPosition / 4 % NUMPIXELS;
  if (newActivePixel < 0) {
    newActivePixel = NUMPIXELS - abs(newActivePixel);
  }

  if (newActivePixel != activePixel) {
    paintPixel(newActivePixel);
  }

}

void paintActivePixel() {
  paintPixel(activePixel);
}

void paintPixel(int newActivePixel) {
    pixels.setPixelColor(activePixel, 0);
    activePixel = newActivePixel;
    pixels.setPixelColor(activePixel, colors[colorIndex]);
    pixels.show();

    u8g2.clearBuffer();
//    do {
      u8g2.setFont(u8g2_font_ncenB14_tr);
      char buffer[3];
      u8g2.drawStr(0,20,itoa(newActivePixel+1, buffer, 10));
      u8g2.drawStr(0,60,colorNames[colorIndex]);
//    } while ( u8g2.nextPage() );
    u8g2.sendBuffer();
}

