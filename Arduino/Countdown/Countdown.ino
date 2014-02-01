// 7 bar segments + 1 dot segment (active low)
const int SEGMENTS = 8;
const int DOT_SEGMENT = SEGMENTS - 1;

// note that the pins are different from the Fritzing sketch
// because my 7 segment display has a different pin assignment
int segmentPins[] = { 2, 3, 4, 5, 6, 7, 8, 9 };
int segmentState[] = { 0, 0, 0, 0, 0, 0, 0, 0  };

const int DIGITS = 11;
const int ALL_OFF = 10;
int digitsState[DIGITS][SEGMENTS - 1] = {
  { 1, 1, 1, 0, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 1, 1 },
  { 1, 1, 0, 1, 1, 1, 0 },
  { 1, 0, 0, 1, 1, 1, 1 },
  { 0, 0, 1, 1, 0, 1, 1 },
  { 1, 0, 1, 1, 1, 0, 1 },
  { 1, 1, 1, 1, 1, 0, 1 },
  { 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1 },
  { 1, 0, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 0 }
};

const int START_PIN = 11;

// push buttons (active low)
int switch1Pin = 12;
int switch2Pin = 13;

boolean switch1Pressed = false;
boolean switch2Pressed = false;

const int INITIAL_COUNTDOWN_COUNT = 9;
int countdownCount = INITIAL_COUNTDOWN_COUNT;
const int START_DURATION = 5;

void setup() {
  for (int i=0; i<SEGMENTS; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  pinMode(START_PIN, OUTPUT);
  pinMode(switch1Pin, INPUT);
  pinMode(switch2Pin, INPUT);

  segmentState[DOT_SEGMENT] = 1;
  writeDigit(countdownCount);
}

void loop() {
  if (digitalRead(switch1Pin) == LOW) {
    switch1Pressed = true;
  } else {
    if (switch1Pressed) {
      countdown();
      switch1Pressed = false;
    }
  }
  
  if (digitalRead(switch2Pin) == LOW) {
    if (!switch2Pressed) {
      switch2Pressed = true;
      countUp();
    }
  } else {
    switch2Pressed = false;
  }
  
  delay(25);
}

void writeDigit(int digitIndex) {
  memcpy(segmentState, digitsState[digitIndex], sizeof(digitsState[digitIndex]));
  for (int i=0; i<SEGMENTS; i++) {
    digitalWrite(segmentPins[i], segmentState[i] == 1 ? LOW : HIGH);
  }
}

void countUp() {
  if (countdownCount == 9) {
    countdownCount = 1;
  } else {
    countdownCount++;
  }
  
  writeDigit(countdownCount);
}

void countdown() {
  segmentState[DOT_SEGMENT] = 0;
  writeDigit(countdownCount);
  
  while (countdownCount > 0) {
    delay(1000);
    countdownCount--;
    writeDigit(countdownCount);
  }
  
  digitalWrite(START_PIN, HIGH);
  
  for (int i=0; i<START_DURATION; i++) {
    delay(500);
    writeDigit(ALL_OFF);
    delay(500);
    writeDigit(0);
  }
  
  digitalWrite(START_PIN, LOW);

  countdownCount = INITIAL_COUNTDOWN_COUNT;
  segmentState[DOT_SEGMENT] = 1;
  writeDigit(countdownCount);
}
