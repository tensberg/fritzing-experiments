import processing.video.*;
import processing.serial.*;

Capture video;
PFont textFont = createFont("Loma Bold",16,true);
PFont bigFont = createFont("Loma Bold",65,true);

CenterText centerText = new CenterText(bigFont);
Race race = new Race("Tensberg", textFont);
Mode mode = Mode.CONFIG;

void setup() {
  size(640, 480);
  
  background(255);

  fill(0);

  frameRate(25);
  
  video = new Capture(this, 640, 480, 25);
  video.start();
  race.startRace();
}

void draw() {
  if (frameCount % 75 == 0) {
    centerText.showText("LAP " + frameCount + "\n01:20:123");
  }
  
  set(0, 0, video);
  race.draw();
  centerText.draw();
}

void captureEvent(Capture c) {
  c.read();
}

