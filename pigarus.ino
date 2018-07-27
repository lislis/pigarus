// Pins
const int fanPin = A0;
const int pigPin = 9;
//const int motorPin = 10; // now using h-bridge
const int motorPull = 6;
const int motorRelease = 5;
const int buttonPin = 8;
const int blueLedPin = 12;
const int yellowLedPin = 11;
const int winSwitchPin = 13;

// LED
unsigned long previousMillis = 0;
const long blinkInterval = 1000;

// Fan
int filterVal = 10; // q'n'd low pass filter
int fanVal;
const int motorThreshold = 90;

// DEBUG
bool debugFan = false;

// crappy state machine
struct GameState {
  bool reset = true;
  unsigned long resetTimer = 30000; // tbd
  unsigned long resetDt = 0;
  bool win = false;
  unsigned long winTimer = 8000; // tbd
  unsigned long winDt = 0;
  bool idle = false;
  bool gameloop = false;
  unsigned long timeScore = 0;
  unsigned long timeScoreDt = 0;
} game;

void setup() {
  Serial.begin(9600);
  pinMode(fanPin, INPUT);
  pinMode(pigPin, OUTPUT);
  //pinMode(motorPin, OUTPUT);
  pinMode(motorPull, OUTPUT);
  pinMode(motorRelease, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(blueLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(winSwitchPin, INPUT_PULLUP);

  digitalWrite(yellowLedPin, HIGH);
  digitalWrite(blueLedPin, HIGH);
  analogWrite(pigPin, 150);
  delay(2500);
  Serial.println("Setup done");
  Serial.println("reset to clean up previous game state");
}

void loop() {
  unsigned long currentMillis = millis();
  switchOffThings();

  if (game.reset) {
    blinkLED(currentMillis);
    motorReleasing();
    countDownResetTimer(currentMillis);
  } else if (game.idle) {
    stopMotor();
    lightLED();
    waitForStartButton(currentMillis);
  } else if (game.gameloop) {
    countUpScoreTime(currentMillis);
    readFan();
    moveMotorAndPig();
    waitForWinSwitch();
  } else if (game.win) {
    stopMotor();
    lightBlueLED();
    countDownWinTimer(currentMillis);
  }

  delay(50);
}

void motorPulling() {
  analogWrite(motorPull, 200);
  digitalWrite(motorRelease, LOW);
}

void motorReleasing() {
  analogWrite(motorRelease, 180);
  digitalWrite(motorPull, LOW);
}

void stopMotor() {
  digitalWrite(motorRelease, LOW);
  analogWrite(motorPull, LOW);
}

void switchOffThings() {
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(blueLedPin, LOW);
  digitalWrite(pigPin, LOW);
  digitalWrite(motorPull, LOW);
  digitalWrite(motorRelease, LOW);
}

void countDownWinTimer(unsigned long cM) {
  if (cM - game.winDt >= game.winTimer) {
    game.winDt = cM;
    game.win = false;
    game.reset = true;
    Serial.println("reset game");
  }
}

void waitForWinSwitch() {
  if (winSwitchPin == 1) {
    game.gameloop = false;
    game.win = true;
    Serial.println("win");
    Serial.println(game.timeScore);
  }
}

void countUpScoreTime(unsigned long cM) {
  game.timeScore = cM - game.timeScoreDt;
}

void waitForStartButton(unsigned long cM) {
  if (digitalRead(buttonPin) == 0) {
    game.idle = false;
    game.gameloop = true;
    game.timeScoreDt = cM;
    Serial.println("start game loop");
  }
}

void lightLED() {
  digitalWrite(yellowLedPin, HIGH);
}

void lightBlueLED() {
  digitalWrite(blueLedPin, HIGH);
}

void countDownResetTimer(unsigned long cM) {
  if (cM - game.resetDt >= game.resetTimer) {
    game.resetDt = cM;
    game.reset = false;
    game.idle = true;
    Serial.println("reset done, idling, PUSH THE BUTTON");
  }
}

void blinkLED(unsigned long currentMillis) {
  int ledState = LOW;
  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;

    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(yellowLedPin, ledState);
  }
}

void readFan() {
  fanVal = analogRead(fanPin);
  filterVal = (fanVal + filterVal) / 2;
  if (debugFan) {
    Serial.print("0, 1023, ");
    Serial.println(filterVal);
  }
}

void moveMotorAndPig() {
  if (filterVal >= motorThreshold) {
    //analogWrite(motorPin, 200);
    motorPulling();
    analogWrite(pigPin, 150);
  } else {
    //digitalWrite(motorPin, LOW);
    stopMotor();
    digitalWrite(pigPin, LOW);
  }
}
