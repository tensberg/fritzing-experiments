#include <Servo.h>
 
Servo head;
const int DEG_MIN = 25;
const int DEG_MAX = 150;
const int FULL_MOVE_TIME = 380;
const int MICROS_PER_DEG = FULL_MOVE_TIME*100 / (DEG_MAX - DEG_MIN);
const int MAX_LIGHT = 1024;

const int LICHTSENSOR = 0;

int lightMinAbs;
int lightSeekPrev;
int lightMaxAbs;

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
  }
  
  moveHead(90);
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
}

void do_pause() {
  int light = readLight();
  if (deviation(light, lightMinAbs) > 30) {
    init_seek(light);
  }
}

void init_seek(int light) {
  Serial.print("seek ");
  Serial.println(light);
  mode = MODE_SEEK;
  turns = 0;
  lightSeekPrev = MAX_LIGHT;
}

void do_seek() {
  int position = head.read();
  if (position <= DEG_MIN || position >= DEG_MAX) {
    boolean abortSeek = do_turn();
    if (abortSeek) {
      moveHead(90);
      init_pause();
      return;
    }
  }
  if (position < DEG_MIN) {
    position = DEG_MIN;
  } else if (position > DEG_MAX) {
    position = DEG_MAX;
  }
  
  int newPosition = position + direction*STEPS;
  moveHead(newPosition);
  
  int light = readLight();
  if (deviation(light, lightMinAbs) < 30) {
    init_pause();
  }
}

boolean do_turn() {
  direction = -direction;
  turns++;
  
  if (turns == 3) {
    lightMinAbs = readLight();
    Serial.print("turn reset to ");
    Serial.println(lightMinAbs);
    turns == 0;
    
    return true;
  }
  
  return false;
}

int deviation(int lightRef, int lightMin) {
  return abs(lightRef - lightMin);
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
