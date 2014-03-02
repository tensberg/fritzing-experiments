// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int photoPin = 0; // analog input of the photoresistor

const int redPin = 8;
const int yellowPin = 9;
const int greenPin = 10;

const int buzzerPin = 6;

const int CALIBRATION = 0;
const int RACE = 1;

int mode = CALIBRATION;

int lap;
long lapStart;
long lastLapTimeTenth;
boolean newLapStarted = false;

long bestTime = 100000;

const int BARRIER_THRESHOLD_LOW = 600;
const int BARRIER_THRESHOLD_HIGH = 800;

const int BARRIER_OPENED = 0;
const int BARRIER_CLOSED = 1;
const int BARRIER_NOCHANGE = 2;
boolean barrierClosed = false;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  lcd.begin(16, 2);
  
  readySetGo();
}

void loop() {
  int light = analogRead(photoPin);
  
  int barrierState = checkBarrierState(light);
  
  switch (mode) {
    case CALIBRATION:
      doCalibration(barrierState, light);
      break;
      
    case RACE:
      doRace(barrierState);
      break;
  }
    
  delay(10);
}

void doCalibration(int barrierState, int light) {
  if (barrierState == BARRIER_CLOSED) {
    startRace();
  } else {
    lcd.setCursor(0, 1);
    lcd.print(light);
    lcd.print("   ");
  }
}

void startRace() {
  mode = RACE;
  lap = 1;
  lapStart = millis();
  digitalWrite(greenPin, LOW);
  tone(buzzerPin, 880, 500);
  lcd.setCursor(0, 0);
  lcd.print("R   1 Z 0.0");
  lcd.setCursor(0, 1);
  lcd.print("B     Z");
}

void doRace(int barrierState) {
  updateLapTime();

  if (barrierState == BARRIER_CLOSED && lastLapTimeTenth >= 3) {
    newLap();
  }
  
  if (newLapStarted && lastLapTimeTenth >= 15) {
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, LOW);
    lcd.setCursor(14,0);
    lcd.print("  ");
    newLapStarted = false;
  }
}

void newLap() {
  long lapEnd = millis();
  long lapTime = lapEnd - lapStart;
  printTime(lapTime, 0, false);
  if (lapTime < bestTime) {
    newBestLap(lap, lapTime);
  } else {
    noBestLap();
  }
  
  lap++;
  printLap(lap, 0);
  lapStart = lapEnd;
  lastLapTimeTenth = -1;
  newLapStarted = true;
}

void updateLapTime() {
  long lapTime = millis() - lapStart;
  long lapTimeTenth = lapTime/100;
  
  if (lapTimeTenth != lastLapTimeTenth) {
    if (!newLapStarted) {
      printTime(lapTime, 0, true);
    }
    lastLapTimeTenth = lapTimeTenth;
  }
}

void newBestLap(int lap, int lapTime) {
  digitalWrite(greenPin, HIGH);
  tone(buzzerPin, 880, 350);
  printLap(lap, 1);
  bestTime = lapTime;
  printTime(bestTime, 1, false);
}

void noBestLap() {
  digitalWrite(redPin, HIGH);
  tone(buzzerPin, 220, 200);
}

void printLap(int lap, int row) {
  int col;
  if (lap >= 100) {
    col = 2;
  } else if (lap >= 10) {
    col = 3;
  } else {
    col = 4;
  }
  lcd.setCursor(col, row);
  lcd.print(lap);
}

void printTime(long time, int row, boolean printTens) {
  long minutesPart = time / 60000;
  long secondsPart = (time % 60000) / 1000;
  long millisPart = time % 1000;

  lcd.setCursor(7, row);

  if (minutesPart < 10) {
    lcd.print(' ');
  }
  lcd.print(minutesPart);
  
  lcd.print(':');

  if (secondsPart < 10) {
    lcd.print('0');
  }
  lcd.print(secondsPart);
  
  lcd.print('.');
  
  if (printTens) {
    lcd.print(millisPart / 100);
  } else {
    if (millisPart < 10) {
      lcd.print("00");
    } else if (millisPart < 100) {
      lcd.print('0');
    }
    lcd.print(millisPart);
  }
}

int checkBarrierState(int light) {
  int state = BARRIER_NOCHANGE;
  
  if (barrierClosed && light >= BARRIER_THRESHOLD_HIGH) {
    barrierClosed = false;
    state = BARRIER_OPENED;
    digitalWrite(yellowPin, LOW);
  } else if (!barrierClosed && light <= BARRIER_THRESHOLD_LOW) {
    barrierClosed = true;
    state = BARRIER_CLOSED;
    digitalWrite(yellowPin, HIGH);
  }
  
  return state;
}

void readySetGo() {
  digitalWrite(redPin, HIGH);
  lcd.print("READY");
  tone(buzzerPin, 440, 100);
  delay(1000);
  
  digitalWrite(yellowPin, HIGH);
  lcd.setCursor(0,0);
  lcd.print("SET  ");
  tone(buzzerPin, 440, 100);
  delay(1000);
  
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  lcd.setCursor(0,0);
  lcd.print("GO   ");
  tone(buzzerPin, 880, 500);
}
