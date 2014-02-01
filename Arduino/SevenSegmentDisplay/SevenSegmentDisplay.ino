// 7 bar segments + 1 dot segment (active low)
const int SEGMENTS = 8;
const int DOT_SEGMENT = SEGMENTS - 1;

// note that the pins are different from the Fritzing sketch
// because my 7 segment display has a different pin assignment
int segmentPins[] = { 2, 3, 4, 5, 6, 7, 8, 9 };
int segmentState[] = { 0, 0, 0, 0, 0, 0, 0, 0  };

const int DIGITS = 10;
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
  { 1, 0, 1, 1, 1, 1, 1 }
};

// push buttons (active low)
int switch1Pin = 12;
int switch2Pin = 13;

boolean switch1Pressed = false;
boolean switch2Pressed = false;

const int MODES = 4;
const int SEGMENT_MODE = 0;
const int DIGIT_MODE = 1;
const int CLOCK_MODE = 2;
const int DICE_MODE = 3;

int mode = SEGMENT_MODE;

int activeSegment = 0;
int activeDigit = 0;
long clockStart;
boolean dotActive = false;
boolean rollDice;

void setup() {
  for (int i=0; i<SEGMENTS; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }

  pinMode(switch1Pin, INPUT);
  pinMode(switch2Pin, INPUT);
  setupMode();
  randomSeed(analogRead(0));
}

void setupMode() {
  switch (mode) {
    case SEGMENT_MODE:
      setupSegmentMode();
      break;
      
    case DIGIT_MODE:
      setupDigitMode();
      break;
      
    case CLOCK_MODE:
       setupClockMode();
       break;
       
    case DICE_MODE:
       setupDiceMode();
       break;   
  }
}

void loop() {
  if (digitalRead(switch1Pin) == LOW) {
    if (!switch1Pressed) {
      switch1Pressed = true;
      action1();
    }
  } else {
    switch1Pressed = false;
  }
  
  if (digitalRead(switch2Pin) == LOW) {
    if (!switch2Pressed) {
      switch2Pressed = true;
      action2();
    }
  } else {
    switch2Pressed = false;
  }
  
  switch (mode) {
    case CLOCK_MODE:
      updateClock();
      break;
    
    case DICE_MODE:
      updateDice();
      break;

      // other modes: no updates
  }
  
  delay(25);
}

void action1() {
  switch (mode) {
    case SEGMENT_MODE:
      toggleSegment();
      break;
      
    case DIGIT_MODE:
      toggleDigit();
      break;
      
    // case CLOCK_MODE: no action
    
    case DICE_MODE:
      toggleRollDice();
      break;
  }
}

void action2() {
  mode = (mode+1) % MODES;
  setupMode();
}

void writeSegmentState() {
  for (int i=0; i<SEGMENTS; i++) {
    digitalWrite(segmentPins[i], segmentState[i] == 1 ? LOW : HIGH);
  }
}

void setupSegmentMode() {
  activeSegment = 0;
  writeActiveSegment();
}

void toggleSegment() {
  activeSegment = (activeSegment+1) % SEGMENTS;
  writeActiveSegment();
}

void writeActiveSegment() {
  for (int i=0; i<SEGMENTS; i++) {
    segmentState[i] = activeSegment == i ? 1 : 0;
  }
  writeSegmentState();
}

void setupDigitMode() {
  activeDigit = 0;
  writeActiveDigit();
}

void toggleDigit() {
  activeDigit = (activeDigit+1) % DIGITS;
  writeActiveDigit();
}

void writeActiveDigit() {
  memcpy(segmentState, digitsState[activeDigit], sizeof(digitsState[activeDigit]));
  writeSegmentState();
}

void setupClockMode() {
  dotActive = false;
  activeDigit = -1; // force update
  clockStart = millis();
  updateClock();
}

void updateClock() {
  int newActiveDigit = (millis()-clockStart) / 1000 % DIGITS;
  if (newActiveDigit != activeDigit) {
    activeDigit = newActiveDigit;
    dotActive = !dotActive;
    segmentState[DOT_SEGMENT] = dotActive ? 1 : 0;
    writeActiveDigit();
  }
}

void setupDiceMode() {
  segmentState[DOT_SEGMENT] = 1;
  rollDice = true;
  updateDice();
}

void updateDice() {
  if (rollDice) {
    activeDigit = random(1, 7);
    writeActiveDigit();
  }
}

void toggleRollDice() {
  rollDice = !rollDice;
  segmentState[DOT_SEGMENT] = rollDice ? 1 : 0;
  writeSegmentState();
}
