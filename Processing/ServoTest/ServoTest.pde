import processing.serial.*;                                                     // Einbinden der Serial Programmbibliothek
import cc.arduino.*;                                                            // Einbinden der Arduino Programmbibliothek

Arduino arduino;

final int servoPin=9;

final int DEG_MIN = 0;
final int DEG_MAX = 180;

int deg = DEG_MIN;

void setup() {
  println(Arduino.list());                                                      // Alle seriellen Geräte werden in einer Liste ausgegeben, die Nummer für das Arduino muss 
  arduino = new Arduino(this, Arduino.list()[0], 57600);                               // hier übergeben werden: Arduino.list()[nummer]
  arduino.pinMode(servoPin, Arduino.SERVO);  // Servo Pin wird im Arduino als Output festgelegt
  arduino.servoWrite(servoPin, 5);
  delay(500);
}

void draw() {
  deg += 5;
  println(deg);
  arduino.servoWrite(servoPin, deg);
  if (deg >= DEG_MAX) {
    deg = DEG_MIN;
  }
  delay(500);
}

