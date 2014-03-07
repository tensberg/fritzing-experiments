import processing.video.*;
import processing.serial.*;

Capture video;

final PFont textFont = createFont("Loma Bold",16,true);
final PFont bigFont = createFont("Loma Bold",45,true);
final CenterText centerText = new CenterText(bigFont);
final Race race = new Race("Tensberg", textFont, centerText);
final String ARDUINO_SERIAL = "/dev/ttyACM0";

Serial arduino;

void setup() {
  size(640, 480);
  
  background(255);

  fill(0);

  frameRate(25);
  
  video = new Capture(this, 640, 480, 25);
  video.start();

  arduino = new Serial(this, ARDUINO_SERIAL, 57600);
  arduino.bufferUntil('\n');
}

void draw() {
  set(0, 0, video);
  race.draw();
  centerText.draw();
}

void captureEvent(Capture c) {
  c.read();
}

void serialEvent(Serial whichPort) {
  String line = arduino.readString();
  line = line.substring(0, line.length() - 2); // strip the CRLF
  
  println(line);
  if (line.equals("READY") || line.equals("SET") || line.equals("GO")) {
    race.reset();
    centerText.showText(line);
  } else if (line.startsWith("START")) {
    race.startRace(Integer.parseInt(line.substring(6)));
  } else if (line.startsWith("LAP")) {
    race.nextLap();
  } else if (line.startsWith("FINISH")) {
    race.finish();
  }
}
