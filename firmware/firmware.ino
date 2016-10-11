/*
 * stepper motor driver
 * 
 * contributers:
 *   Blaise Thompson - blaise@untzag.com
 *   
 * last modified 2016-10-09
 *
 *  The MIT License (MIT)
 *  
 *  Copyright (c) 2016 John C Wright
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
 *  documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 *  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions
 *  of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 *  TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 *  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */


// pins
int S0 = A0;
int S1 = A1;
int S2 = A2;
int CLR = 8;
int G = 9;
int D = 10;
int M0 = A4;
int M1 = A5;
int HOME = 11;
int motors[6] = {2, 3, 4, 5, 6, 7};

// variables
int num = 0;
#define INPUT_SIZE 100  // TODO: make this a reasonable value
#define sep " "
char input[INPUT_SIZE + 1];
char code = '0';
char c_index = '0';
char c_number = '0';
int index = 0;
int number = 0;

void setup() {
  // initialize pins
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(CLR, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(HOME, INPUT);
  for (int i = 0; i < 6; i += 1) {
    pinMode(motors[i], OUTPUT);
  }
  // initialize select
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
  // initialize microstepping
  digitalWrite(M0, LOW);
  digitalWrite(M1, LOW);
  // initialize direction
  digitalWrite(D, LOW);
  
  // initiate serial
  Serial.begin(57600);
  while (!Serial);  // wait for port to connect
  Serial.println("ready");
}

void loop() {
  if (Serial.available()>0){
    // read serial into input char array
    byte size = Serial.readBytes(input, INPUT_SIZE);
    input[size] = 0;
    // parse input
    char *code = strtok(input, sep);
    char *c_index = strtok(0, sep);
    char *c_number = strtok(0, sep);
    // convert input
    index = atoi(c_index);
    number = atoi(c_number);
    Serial.println(index);
    // call appropriate function
    if (*code == 'M') moveMotor(index, number);
  }
  delay(100);  // speed limiter
  Serial.flush();
}

int getHome(int index){
  for (int i=0; i<=5; i++) {
    setSelect(i);
    Serial.print(i);
    Serial.println(digitalRead(HOME));
  }
}

void moveMotor(int index, int steps) {
  setSelect(index);
  // choose direction
  if (steps >= 0) setDirection(1);
  else if (steps <= 0) setDirection(-1), steps *= -1;
  else delay(100);  // if steps = 0...
  // move
  for (int i=0; i<=steps; i++){
    stepMotor(index);
  }
}

void setDirection(int dir) {
  // TODO
  delay(100);
}
  
void setSelect(int index) {
  digitalWrite(S0, bitRead(index, 0));
  digitalWrite(S1, bitRead(index, 1));
  digitalWrite(S2, bitRead(index, 2));
}

void stepMotor(int index) {
  digitalWrite(motors[index], HIGH);
  delay(5);
  digitalWrite(motors[index], LOW);
  delay(5);
}










