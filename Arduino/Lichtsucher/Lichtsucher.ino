#include <Servo.h>
 
Servo head;
const int DEG_MIN = 25;
const int DEG_MAX = 150;
const int FULL_MOVE_TIME = 380;
const int MICROS_PER_DEG = FULL_MOVE_TIME*100 / (DEG_MAX - DEG_MIN);

const int LICHTSENSOR = 0;

int lightMinAbs;
int lightMaxAbs;
int lightMinCurrent;
int lightMinDeg = DEG_MIN;

const int MODE_SEEK = 0;
const int MODE_PAUSE = 1;
const int MODE_STARTUP= 2;
int mode = MODE_STARTUP;

const int STEPS = 2;
const int DELAY_READ = 5;
int direction = 1;
int turns = 0;
 
void setup() 
{ 
  head.attach(9);
  Serial.begin(9600);
  reference();
} 
 
void loop() 
{ 
  switch (mode) {
    case MODE_STARTUP:
      do_startup();
      break;
    
    case MODE_PAUSE:
      do_pause();
      break;
      
    case MODE_SEEK:
      do_seek();
      break;
  }
}

void reference() {
  lightMinAbs = readLight();
  lightMaxAbs = lightMinAbs;

  head.write(DEG_MIN);
  delay(FULL_MOVE_TIME);
  
  head.write(DEG_MAX);
  for (int deg=DEG_MIN; deg<DEG_MAX; deg+=STEPS) {
    head.write(deg);
    delayMicroseconds(MICROS_PER_DEG*STEPS);
    int light = readLight();
    if (light == lightMinAbs) {
      lightMinDeg = deg;
    }
  }
  
  moveHead(90);
  lightMinCurrent = lightMinAbs;
}

void do_startup() {
  Serial.print("startup ");
  int light = readLight();
  Serial.println(light);
  
  if (light < 300) {
    init_pause();
  } else {
    delay(200);
  }
} 

void init_pause() {
  Serial.println("pause");
  Serial.print("lightMinAbs ");
  Serial.print(lightMinAbs);
  Serial.print(" lightMaxAbs ");
  Serial.print(lightMaxAbs);
  Serial.print(" light ");
  Serial.println(readLight());
  mode = MODE_PAUSE;
  delay(1000);
}

void do_pause() {
  int light = readLight();
  if (light < lightMinCurrent) {
    lightMinCurrent = light;
  } else if (deviationLargerThan(light, lightMinAbs, lightMaxAbs, 5)) {
    init_seek(light);
  }
}

void init_seek(int light) {
  Serial.println("seek");
  lightMinCurrent = light;
  mode = MODE_SEEK;
  turns = 0;
}

void do_seek() {
  int position = head.read();
  if (position <= DEG_MIN || position >= DEG_MAX) {
    do_turn();
  }
  if (position < DEG_MIN) {
    position = DEG_MIN;
  } else if (position > DEG_MAX) {
    position = DEG_MAX;
  }
  
  int newPosition = position + direction*STEPS;
  moveHead(newPosition);
  
  int light = readLight();
  if (light < lightMinCurrent) {
    lightMinDeg = newPosition;
    lightMinCurrent = light;
  } else if (deviationSmallerThan(light, lightMinAbs, lightMaxAbs, 2)) {
    //moveHead(lightMinDeg);
    init_pause();
  }
}

void do_turn() {
  direction = -direction;
  turns++;
  if (turns == 2) {
    lightMinAbs = lightMinCurrent;
    turns == 0;
  }
}

boolean deviationLargerThan(int lightRef, int lightMin, int lightMax, int maxDeviationPercent) {
  int deviation = abs(lightRef - lightMin);
  int deviationPercent = deviation*100 / (lightMax-lightMin);
  
  return deviationPercent > maxDeviationPercent;
}

boolean deviationSmallerThan(int lightRef, int lightMin, int lightMax, int maxDeviationPercent) {
  int deviation = abs(lightRef - lightMin);
  int deviationPercent = deviation*100 / (lightMax-lightMin);
  
  return deviationPercent < maxDeviationPercent;
}

int readLight() {
  delay(DELAY_READ);
  int light = analogRead(LICHTSENSOR);
  lightMinAbs = min(light, lightMinAbs);
  lightMaxAbs = max(light, lightMaxAbs);
  return light;
}

void moveHead(int deg) {
  int position = head.read();
  if (position != deg) {
    head.write(deg);
    delayMicroseconds(abs(deg-position)*MICROS_PER_DEG);
  }
}
