/* multiplexed stepper motor driver
 *
 * stepper controller boards use DRV8834 or similar
 * TI CD74HC4351 multiplexes for steps and microsteps
 * TI SN74HC259 latches motor direction
 *
 * last modified 2026-01-21
 *
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
int u = 1;  // microstepping amount
int selected = 0; // selected motor
#define INPUT_SIZE 100  // TODO: make this a reasonable value
#define sep " "
char input[INPUT_SIZE + 1];
char code = '0';
char c_index = '0';
char c_number = '0';
int index = 0;
int number = 0;
int remaining[6];
bool home_after[6] = {false, false, false, false, false, false};
unsigned long prev = 0;
int interval = 5;  // wait time for main loop; can be extended if needed
int default_interval = 5;  
int homed = false;
int ms_step = 2;  // delay between up and down steps

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
  Serial.begin(57600);
  while (!Serial);  // do nothing, waiting for port to connect
}

void loop() {
  // each loop we check all motors and perform a step on each line if needed

  // interval comparison avoids overflow issues
  unsigned long waited = millis() - prev;

  if (waited >= interval){
    prev += waited;
    interval = default_interval;  // reset interval
    for (i = 0; i <= 5; i++){
      if (remaining[i] > 0) {
        stepMotor(i);
        --remaining[i];
      }
      else if (home_after[i]) { // ready to home after moving off interrupt
        setSelect(i);
        setDirection(HIGH);
        home_after[i] = false;
        remaining[i] = -1;
      }
      else if (remaining[i] == -1) {  // motor currently homing
        stepMotor(i);
        // interrupt is low when blocked
        setSelect(i);
        // read homed twice to make sure
        if (carefulReadHome()) remaining[i] = 0;
      }
    }
  }
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
    if (carefulReadHome()) {  // interrupt is low when blocked
      // issue instruction to move 1/4 turn counter-clockwise
      setDirection(LOW);
      remaining[index] = 100 * u;
      // set flag to home after this movement is done
      home_after[index] = true;
    }
    // if interrupt is not blocked, we set remaining to -1, a special code for home
    else {
      setDirection(HIGH);
      remaining[index] = -1;
    }
  }
  else if (*code == 'Q') {  // query motor status
    // // ignore faults for now
    // setSelect(index);
    // if (digitalRead(FAULT) == 0) Serial.println('F');
    if (remaining[index] == 0 and not home_after[index]) Serial.println('R');
    else Serial.println('B');
  }
  else if (*code == '?') {
    setSelect(index);
    if (digitalRead(HOME)==0) Serial.println('H');
    else Serial.println('N');
    Serial.println(String(remaining[index]));
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
  // facing the motor...
  //   LOW = counter clockwise
  //   HIGH = clockwise
  digitalWrite(G, LOW);  // allow value to be written
  digitalWrite(D, dir);
  digitalWrite(G, HIGH);  // lock in value
}

void setSelect(int index) {
  // selection reveals HOME and DIR pins for a specific motor
  if (index != selected) {
    digitalWrite(S0, bitRead(index, 0));
    digitalWrite(S1, bitRead(index, 1));
    digitalWrite(S2, bitRead(index, 2));
    selected = index;
  }
}

bool carefulReadHome() {
  // blocking, careful measurement of HOME status
  // wait for 5 consistent measurements (with delay)
  while (true) {
    homed = 0;
    for (i = 0; i < 5; i++){
      delay(1);
      homed += digitalRead(HOME);
      Serial.println(String(homed));
    }
    if (homed % 5 == 0) break;
    else Serial.println("retry");  // how often does this happen?
  }
  if (homed == 5) Serial.println("I am not homed!");
  else Serial.println("I am homed!");
  return (homed == 0);
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
  delay(ms_step);
  digitalWrite(motors[index], LOW);
  // ensure we have a minimum delay equal to the delay between HIGH and LOW writing
  // BUG: if we are already late, we may not actually add enough to interval
  if ((millis() - prev) <= ms_step) interval = (millis() - prev) + ms_step;
}

