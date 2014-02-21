// Move the Friting creator kit cardboard bots head
// to track the watcher.
// Uses the the OpenCV for Processing library by Greg Borenstein
// and the Arduino library.
// This source is adapted from the LiveCamTest example.
// see http://fritzing.org/creatorkit
// see https://github.com/atduskgreg/opencv-processing/

import gab.opencv.*;
import processing.video.*;
import java.awt.*;
import processing.serial.*;
import cc.arduino.*;

Capture video;
OpenCV opencv;
Arduino arduino;

final int servoPin = 9;

final int DEG_MIN = 45;
final int DEG_MAX = 115;

void setup() {
  arduino = new Arduino(this, Arduino.list()[0], 57600);
  arduino.pinMode(servoPin, Arduino.SERVO);
  arduino.servoWrite(servoPin, 0);
  delay(500);

  size(640, 480);
  video = new Capture(this, 640/2, 480/2);
  opencv = new OpenCV(this, 640/2, 480/2);
  opencv.loadCascade(OpenCV.CASCADE_FRONTALFACE);  

  video.start();
}

void draw() {
  scale(2);
  opencv.loadImage(video);

  image(video, 0, 0 );

  noFill();
  stroke(0, 255, 0);
  strokeWeight(3);
  Rectangle[] faces = opencv.detect();

  for (int i = 0; i < faces.length; i++) {
    rect(faces[i].x, faces[i].y, faces[i].width, faces[i].height);
  }
  
  if (faces.length > 0) {
    lookAtFace(faces[0]);
  }
}

void lookAtFace(Rectangle face) {
  int centerX = face.x + face.width / 2;
  int deg = (int) map(centerX, 0, 320, DEG_MIN, DEG_MAX);
  arduino.servoWrite(servoPin, deg);
}

void captureEvent(Capture c) {
  c.read();
}
