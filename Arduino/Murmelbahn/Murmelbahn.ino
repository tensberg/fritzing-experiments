const int BUTTON_PIN = 2;

const int MOTOR_SPEED_PIN = 3;
const int MOTOR_A_PIN = 4;
const int MOTOR_B_PIN = 5;

const int TOP_SWITCH_PIN = 6;
const int BOTTOM_SWITCH_PIN = 7;

const int GREEN_1_PIN = 8;
const int RED_1_PIN = 9;
const int GREEN_2_PIN = 10;
const int RED_2_PIN = 11;

boolean buttonWasPressed = false;
boolean readyToLaunch = false;
boolean launched = false;
long launchTime;

long bottomSwitchPressed;

void setup() {
  pinMode(GREEN_1_PIN, OUTPUT);
  pinMode(RED_1_PIN, OUTPUT);
  pinMode(GREEN_2_PIN, OUTPUT);
  pinMode(RED_2_PIN, OUTPUT);

  pinMode(MOTOR_SPEED_PIN, OUTPUT);
  pinMode(MOTOR_A_PIN, OUTPUT);
  pinMode(MOTOR_B_PIN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(BOTTOM_SWITCH_PIN, INPUT);
  pinMode(TOP_SWITCH_PIN, INPUT);
 
  setLed(GREEN_2_PIN, false);
  setLed(RED_2_PIN, true);
  
  digitalWrite(MOTOR_A_PIN, HIGH);
  digitalWrite(MOTOR_B_PIN, LOW);
  motorOn();
  
  Serial.begin(57600);
}

void loop() {
  checkTopSwitch();
  checkBottomSwitch();
  checkButton();
  
  delay(10);
}

void checkTopSwitch() {
  if (!readyToLaunch && !launched) {
    boolean topOn = readButton(TOP_SWITCH_PIN);
    if (topOn) {
      motorOff();
      readyToLaunch = true;
      Serial.print("READY");
      Serial.println();
    }
  }
}

void checkBottomSwitch() {
  if (launched) {
    boolean bottomOn = readButton(BOTTOM_SWITCH_PIN);
    if (bottomOn) {
      finish();
    }
  }
}

void checkButton() {
  if (readyToLaunch && !launched) {
    boolean buttonOn = readButton(BUTTON_PIN);
    if (buttonOn) {
      launch();
    }
  }
}

void launch() {
  launchTime = millis();
  readyToLaunch = false;
  launched = true;
  motorOn();
  setLed(GREEN_2_PIN, true);
  setLed(RED_2_PIN, false);
  
  Serial.print("START");
  Serial.println();  
}

void finish() {
  long raceTime = millis() - launchTime;
  
  setLed(GREEN_2_PIN, false);
  setLed(RED_2_PIN, true);
  launched = false;
  
  Serial.print("FINISH ");
  serialPrintTime(raceTime);
  Serial.println();
}

void motorOn() {
  setMotor(true);
  setLed(GREEN_1_PIN, true);
  setLed(RED_1_PIN, false);
}

void motorOff() {
  setMotor(false);
  setLed(GREEN_1_PIN, false);
  setLed(RED_1_PIN, true);
}

void setMotor(boolean on) {
  analogWrite(MOTOR_SPEED_PIN, on ? 255 : 0);
}

void setLed(int pin, boolean on) {
  digitalWrite(pin, on ? HIGH : LOW);
}

boolean readButton(int pin) {
  return digitalRead(pin) == LOW; // active low wiring
}

void serialPrintTime(long time) {
  long minutesPart = time / 60000;
  long secondsPart = time / 1000 % 60;
  long millisPart = time % 1000;

  Serial.print(minutesPart);

  Serial.print(':');

  if (secondsPart < 10) {
    Serial.print('0');
  }
  Serial.print(secondsPart);

  Serial.print('.');
  
  if (millisPart < 10) {
    Serial.print("00");
  } else if (millisPart < 100) {
    Serial.print('0');
  }
  Serial.print(millisPart);
}

