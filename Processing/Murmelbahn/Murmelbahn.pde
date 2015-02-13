import processing.video.*;
import processing.serial.*;

final PFont textFont = createFont("Loma Bold",8,true);
final PFont bigFont = createFont("Loma Bold",45,true);
final CenterText centerText = new CenterText(bigFont);
final Race race = new Race(textFont, centerText);
final String ARDUINO_SERIAL = "/dev/ttyACM0";
final String[] WEBCAMS = { "/dev/video1" /*, "/dev/video1" */};
final int FRAME_RATE = 30;
final String VIDEO_FILE = System.getProperty("user.home") + "/Murmelbahn.mp4";
//final String VIDEO_FILE = "http://localhost:8090/feed1.ffm";

VideoStreamer videoStreamer;
int videoSwitchedLaps = 0;
int currentVideo = 0;
Capture video;
final Capture[] videos = new Capture[WEBCAMS.length];
Serial arduino;

void setup() {
  size(640, 480);
  frameRate(FRAME_RATE);
    
  background(255);
  fill(0);

  for (int i=0; i<WEBCAMS.length; i++) {
    videos[i] = new Capture(this, width, height, WEBCAMS[i], FRAME_RATE);
    videos[i].start();
  }
  video = videos[currentVideo];

  arduino = new Serial(this, ARDUINO_SERIAL, 57600);
  arduino.bufferUntil('\n');

  videoStreamer = new VideoStreamer(VIDEO_FILE, width, height, FRAME_RATE);
  
  Runtime.getRuntime().addShutdownHook(new Thread(new Runnable() {
    public void run () {
      videoStreamer.close();
    }
  }));
}

void draw() {
  set(0, 0, video);
  race.draw();
  centerText.draw();
  videoStreamer.captureDisplayWindow();
}

void stop() {
  videoStreamer.close();
}

void captureEvent(Capture c) {
  c.read();
}

void serialEvent(Serial whichPort) {
  String line = arduino.readString();
  line = line.substring(0, line.length() - 2); // strip the CRLF
  
  println(line);
  if (line.equals("READY")) {
    race.ready();
    centerText.showText(line);
  } else if (line.startsWith("START")) {
    race.startRace();
  } else if (line.startsWith("FINISH")) {
    race.finish();
    video = videos[0];
  }
}
