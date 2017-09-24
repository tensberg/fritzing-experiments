// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      60

#define MAXACTIVEPIXELS 10

#define MAXBRIGHTNESS 50

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int activePixels = -1;
int brightness = -1;

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
}

void loop() {

  delay(50);

  int activePixelsSetting = analogRead(1);
  int brightnessSetting = analogRead(0);

  int newActivePixels = map(activePixelsSetting, 0, 1000, 1, MAXACTIVEPIXELS);
  int newBrightness = map(brightnessSetting, 0, 1000, 0, MAXBRIGHTNESS);

  if (newActivePixels != activePixels || newBrightness != brightness) {
    activePixels = newActivePixels;
    brightness = newBrightness;

    for (int pixel=0; pixel<activePixels; pixel++) {
      pixels.setPixelColor(pixel, pixels.Color(brightness, brightness,brightness));
    }

    for (int pixel=activePixels; pixel<NUMPIXELS; pixel++) {
      pixels.setPixelColor(pixel, 0);
    }

    pixels.show();
  }

}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
