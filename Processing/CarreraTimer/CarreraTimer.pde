import processing.video.*;
import processing.serial.*;

final PFont textFont = createFont("Loma Bold",16,true);
final PFont bigFont = createFont("Loma Bold",45,true);
final CenterText centerText = new CenterText(bigFont);
final Race race = new Race("Tensberg", textFont, centerText);
final String ARDUINO_SERIAL = "/dev/ttyACM0";
final String[] WEBCAMS = { "/dev/video0", "/dev/video1" };
final int FRAME_RATE = 30;

Capture video;
final Capture[] videos = new Capture[WEBCAMS.length];
Serial arduino;

void setup() {
  size(640, 480);
  
  background(255);

  fill(0);

  frameRate(FRAME_RATE);
  
  for (int i=0; i<WEBCAMS.length; i++) {
    videos[i] = new Capture(this, 640, 480, WEBCAMS[i], FRAME_RATE);
    videos[i].start();
  }
  video = videos[0];

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
    if (random(1) < 0.3) {
      video = videos[(int) random(0, videos.length)];
    }
  } else if (line.startsWith("FINISH")) {
    race.finish();
  }
}
