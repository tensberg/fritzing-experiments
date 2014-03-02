// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int photoPin = 0; // analog input of the photoresistor

const int RED_PIN = 8; // red LED
const int YELLOW_PIN = 9; // yellow LED
const int GREEN_PIN = 10; // green LED

const int buzzerPin = 6;

const int SETUP = 0;
const int RACE = 1;
const int FINISH = 2;

int mode = SETUP;

const int TOTAL_LAP_INCREMENT = 25;

long raceStart;
long totalLaps = TOTAL_LAP_INCREMENT;
int lap;
long lapStart;
long lastLapTime;
long lastTotalTimeSeconds;
boolean newLapStarted = false;

long bestTime = 100000;

const int BARRIER_THRESHOLD_LOW = 600;
const int BARRIER_THRESHOLD_HIGH = 800;

const int BARRIER_OPENED = 0;
const int BARRIER_CLOSED = 1;
const int BARRIER_NOCHANGE = 2;
boolean barrierClosed = false;

const int MINIMUM_LAP_TIME_MILLIS = 300;
const int LAST_LAP_DISPLAY_MILLIS = 1500;

const int COL_LAP_TOTAL = 1;
const int COL_LAP_TIME = 7;

const int PRECISION_SECONDS = 0;
const int PRECISION_TENS = 1;
const int PRECISION_MILLIS = 2;

const int SHOW_LAP = 0;
const int SHOW_TOTAL_TIME = 1;
int lapTimeMode;

const char TOTAL_LAPS_MARKER = 'R';
const char TOTAL_TIME_MARKER = 'G';
const char FASTEST_LAP_MARKER = 'B';
const char LAP_TIME_MARKER = 'Z';

boolean ledsOn;

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  lcd.begin(16, 2);
  
  readySetGo();
}

void loop() {
  int light = analogRead(photoPin);
  
  int barrierState = checkBarrierState(light);
  
  switch (mode) {
    case SETUP:
      doCalibration(barrierState, light);
      break;
      
    case RACE:
      doRace(barrierState);
      break;
      
    case FINISH:
      doFinish();
      break;
  }
    
  delay(5);
}

void readySetGo() {
  setLed(RED_PIN, true);
  lcd.print("READY");
  tone(buzzerPin, 440, 100);
  delay(1000);
  
  setLed(YELLOW_PIN, true);
  lcd.setCursor(0,0);
  lcd.print("SET  ");
  tone(buzzerPin, 440, 100);
  delay(1000);
  
  setLed(GREEN_PIN, true);
  setLed(RED_PIN, false);
  setLed(YELLOW_PIN, false);
  lcd.setCursor(0,0);
  lcd.print("GO   ");
  tone(buzzerPin, 880, 500);
}

int checkBarrierState(int light) {
  int state = BARRIER_NOCHANGE;
  
  if (barrierClosed && light >= BARRIER_THRESHOLD_HIGH) {
    barrierClosed = false;
    state = BARRIER_OPENED;
    setLed(YELLOW_PIN, false);
  } else if (!barrierClosed && light <= BARRIER_THRESHOLD_LOW) {
    barrierClosed = true;
    state = BARRIER_CLOSED;
    setLed(YELLOW_PIN, true);
  }
  
  return state;
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
  lapTimeMode = SHOW_LAP;
  lap = 1;
  raceStart = millis();
  lapStart = raceStart;
  lastTotalTimeSeconds = 0;

  setLed(GREEN_PIN, false);
  tone(buzzerPin, 880, 500);
  lcd.setCursor(0, 0);
  lcd.print(TOTAL_LAPS_MARKER);
  lcd.print("   1 ");
  lcd.print(LAP_TIME_MARKER);
  lcd.print("0:00.0");
  lcd.setCursor(0, 1);
  lcd.print(FASTEST_LAP_MARKER);
  lcd.print("     ");
  lcd.print(LAP_TIME_MARKER);
}

void doRace(int barrierState) {
  long now = millis();
  updateLapTime(now);
  updateTotalTime(now);

  if (barrierState == BARRIER_CLOSED && lastLapTime >= MINIMUM_LAP_TIME_MILLIS) {
    newLap(now);
  }
  
  if (newLapStarted && lastLapTime >= LAST_LAP_DISPLAY_MILLIS) {
    setLed(GREEN_PIN, false);
    setLed(RED_PIN, false);
    lcd.setCursor(COL_LAP_TIME + 7,0);
    lcd.print("  ");
    newLapStarted = false;
  }
}

void newLap(long now) {
  long lapTime = now - lapStart;
  printTime(lapTime, COL_LAP_TIME, 0, PRECISION_MILLIS);
  if (lapTime < bestTime) {
    newBestLap(lap, lapTime);
  } else {
    noBestLap();
  }

  if (lap == totalLaps) {
    finish(now);
  } else {
    lap++;
    printLap(lap, 0);
    lapStart = now;
    lastLapTime = -1;
    newLapStarted = true;
  }
}

void updateLapTime(long now) {
  long lapTime = now - lapStart;
  long lapTimeTenth = lapTime/100;
  
  if (lapTimeTenth != lastLapTime / 100) {
    if (!newLapStarted) {
      printTime(lapTime, COL_LAP_TIME, 0, PRECISION_TENS);
    }
    lastLapTime = lapTime;
  }
}

void newBestLap(int lap, int lapTime) {
  digitalWrite(GREEN_PIN, HIGH);
  tone(buzzerPin, 880, 350);
  printLap(lap, 1);
  bestTime = lapTime;
  printTime(bestTime, COL_LAP_TIME, 1, PRECISION_MILLIS);
}

void noBestLap() {
  digitalWrite(RED_PIN, HIGH);
  tone(buzzerPin, 220, 200);
}

void updateTotalTime(long now) {
  long totalTime = now - raceStart;
  
  if ((totalTime / 3000) % 2 == 0) {
    if (lapTimeMode != SHOW_LAP) {
      switchLapMode();
    }
  } else {
    long totalTimeSeconds = totalTime / 1000;
    if (lapTimeMode != SHOW_TOTAL_TIME) {
      switchTotalTimeMode(totalTime);
    } else if (lastTotalTimeSeconds < totalTimeSeconds) {
      showTotalTime(totalTime);
    }
    lastTotalTimeSeconds = totalTimeSeconds;
  }
}

void switchLapMode() {
  lcd.setCursor(0, 0);
  lcd.print(TOTAL_LAPS_MARKER);
  printLap(lap, 0);
  lapTimeMode = SHOW_LAP;
}

void switchTotalTimeMode(long totalTime) {
  lcd.setCursor(0, 0);
  lcd.print(TOTAL_TIME_MARKER);
  lapTimeMode = SHOW_TOTAL_TIME;
  showTotalTime(totalTime);
}

void showTotalTime(long totalTime) {
  printTime(totalTime, COL_LAP_TOTAL, 0, PRECISION_SECONDS);
}

void finish(long now) {
  mode = FINISH;
  long totalTime = now - raceStart;
  
  // print laps and total time in row 0
  switchLapMode();
  lcd.setCursor(COL_LAP_TIME - 1, 0);
  lcd.print(TOTAL_TIME_MARKER);
  printTime(totalTime, COL_LAP_TIME, 0, PRECISION_MILLIS);
  
  tone(buzzerPin, 440, 3000);
  
  ledsOn = true;
}

void doFinish() {
  setLed(GREEN_PIN, ledsOn);
  setLed(YELLOW_PIN, ledsOn);
  setLed(RED_PIN, ledsOn);
  ledsOn = !ledsOn;
  delay(1000);
}

void setLed(int pin, boolean on) {
  digitalWrite(pin, on ? HIGH : LOW);
}

void printLap(int lap, int row) {
  int col;
  lcd.setCursor(1, row);
  if (lap >= 100) {
    lcd.print(' ');
  } else if (lap >= 10) {
    lcd.print("  ");
  } else {
    lcd.print("   ");
  }
  lcd.print(lap);
  lcd.print(' ');
}

void printTime(long time, int col, int row, int precision) {
  long minutesPart = time / 60000;
  long secondsPart = (time % 60000) / 1000;
  long millisPart = time % 1000;

  lcd.setCursor(col, row);

  if (minutesPart < 10) {
    lcd.print(' ');
  }
  lcd.print(minutesPart);
  
  lcd.print(':');

  if (secondsPart < 10) {
    lcd.print('0');
  }
  lcd.print(secondsPart);
  
  if (precision != PRECISION_SECONDS) {
    
    lcd.print('.');
  
    if (precision == PRECISION_TENS) {
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
}

