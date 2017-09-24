/*
 * Measure battery voltage and draw it on a screen
 * connected to TV video in
 */

#include <TVout.h>
#include <fontALL.h>

const unsigned int MAX_SPANNUNG = 328; // -1,6V
const unsigned char TV_BREITE = 128;
const unsigned char TV_HOEHE = 96;

TVout TV;

void setup() {
  TV.begin(PAL, TV_BREITE, TV_HOEHE);
  TV.select_font(font6x8);
}

void loop() {
  TV.clear_screen();
  unsigned int spannung = analogRead(A0);
  spannung = min(spannung, MAX_SPANNUNG);
  zeichne_batterie(spannung);
  TV.print(8, 80, "A0:");
  TV.print(28, 80, spannung);
  TV.print(70, 80, "V:");
  TV.print(84, 80, spannung * 5.0 / 1024);
  TV.delay_frame(60);
}

void zeichne_batterie(unsigned int spannung) {
  TV.draw_rect(7, 40, 100, 36, WHITE);
  TV.draw_rect(107, 48, 6, 20, WHITE, WHITE);
  unsigned int breite = map(spannung, 0, MAX_SPANNUNG, 0, 100);
  TV.draw_rect(7, 40, breite, 36, WHITE, WHITE);
}
