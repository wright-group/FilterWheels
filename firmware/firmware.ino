/* stepper motor driver
 *
 * contributers:
 *   Blaise Thompson - blaise@untzag.com
 *
 * last modified 2016-10-13
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
int FAULT = 12;
int motors[6] = {2, 3, 4, 5, 6, 7};

// variables
int i = 0;  // for looping
int num = 0;
int u = 1;  // microstepping amount
#define INPUT_SIZE 100  // TODO: make this a reasonable value
#define sep " "
char input[INPUT_SIZE + 1];
char code = '0';
char c_index = '0';
char c_number = '0';
int index = 0;
int number = 0;
int remaining[6];

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
  pinMode(HOME, INPUT_PULLUP);
  pinMode(FAULT, INPUT_PULLUP);
  for (int i = 0; i < 6; i += 1) {
    pinMode(motors[i], OUTPUT);
  }
  // initialize select
  digitalWrite(S0, LOW);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
  // initialize latch
  digitalWrite(CLR, HIGH);
  digitalWrite(G, HIGH);
  // initialize microstepping
  setM();
  // initialize direction
  digitalWrite(D, LOW);
  // initiate serial
  Serial.begin(57600);
  while (!Serial);  // do nothing, waiting for port to connect
}

void loop() {
  for (i = 0; i <= 5; i++){
    setSelect(i);
    if (remaining[i] > 0) stepMotor(i), --remaining[i];
    if (remaining[i] == -1) {  // motor currently homing
      stepMotor(i);
      // interrupt is low when blocked
      if (digitalRead(HOME) == 0) remaining[i] = 0;
    }
  }
  delay(5);
  //Serial.println(millis());
}

void serialEvent() {  // occurs whenever new data comes in the hardware serial RX
  // read serial into input char array
  byte size = Serial.readBytesUntil('\n', input, INPUT_SIZE);
  input[size] = 0;
  // parse input
  char *code = strtok(input, sep);
  char *c_index = strtok(0, sep);
  char *c_number = strtok(0, sep);
  // convert input
  index = atoi(c_index);
  number = atoi(c_number);  // positive = clockwise (facing motor)
  if (index > 32) {
    Serial.println("INVALID");
    return;
  }
  // call appropriate function
  if (*code == 'M') {  // move relative
    setSelect(index);
    if (number > 0) setDirection(HIGH);
    if (number < 0) setDirection(LOW);
    remaining[index] = abs(number);
  }
  else if (*code == 'H') {  // home motor
    // home is defined as the location where the interrupt 
    //   is first tripped when approaching clockwise
    // first, we must handle the special case where the motor
    //   is already at the interrupt
    setSelect(index);
    if (digitalRead(HOME) == 0) {  // interrupt is low when blocked
      // move 1/4 turn counter-clockwise
      setDirection(LOW);
      for (i = 0; i <= 200*u; i++) {
        stepMotor(index);
        delay(5);
      }
    }
    // now we set remaining to -1, a special code for home
    setDirection(HIGH);
    remaining[index] = -1;
  }
  else if (*code == 'Q') {  // query motor status
    setSelect(index);
    if (digitalRead(FAULT) == 0) Serial.println('F');
    else if (remaining[index] == 0) Serial.println('R');
    else Serial.println('B');
  }
  else if (*code == 'U') {  // set microstep integer
    u = index;
    setM();
  }
}

void serialEventRun(void) {
  // this runs inside of the main loop, essentially
  if (Serial.available()) serialEvent();
}

void setDirection(int dir) {
  // direction is latched in using TI SN74HC259
  // facing the motor...
  //   LOW = counter clockwise
  //   HIGH = clockwise
  digitalWrite(G, LOW);  // allow value to be written
  digitalWrite(D, dir);
  digitalWrite(G, HIGH);  // lock in value
}

void setSelect(int index) {
  digitalWrite(S0, bitRead(index, 0));
  digitalWrite(S1, bitRead(index, 1));
  digitalWrite(S2, bitRead(index, 2));
}

void setM() {
  // set the microstepping control pins
  // for these controllers M0 uses three-state logic
  if (u == 1) {
    pinMode(M0, OUTPUT);
    digitalWrite(M0, LOW);
    digitalWrite(M1, LOW);
  }
  else if (u == 2) {
    pinMode(M0, OUTPUT);
    digitalWrite(M0, HIGH);
    digitalWrite(M1, LOW);
  }
  else if (u == 4) {
    pinMode(M0, INPUT);
    digitalWrite(M1, LOW);
  }
  else if (u == 8) {
    pinMode(M0, OUTPUT);
    digitalWrite(M0, LOW);
    digitalWrite(M1, HIGH);
  }
  else if (u == 16) {
    pinMode(M0, OUTPUT);
    digitalWrite(M0, HIGH);
    digitalWrite(M1, HIGH);
  }
  else if (u == 32) {
    pinMode(M0, INPUT);
    digitalWrite(M1, HIGH);
  }
  else {
    Serial.println("INVALID");
    u = 1;
    setM();
  } 
}

void stepMotor(int index) {
  digitalWrite(motors[index], HIGH);
  delay(1);
  digitalWrite(motors[index], LOW);
}

