// Pins
const int fanPin = A0;
const int pigPin = 9;
const int motorPin = 10;
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
const int motorThreshold = 100;

// Serial output
bool debug = true;

// crappy state machine
struct GameState {
  bool reset = true;
  unsigned long resetTimer = 10000; // tbd
  unsigned long resetDt = 0;
  bool win = false;
  unsigned long winTimer = 5000; // tbd
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
  pinMode(motorPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(blueLedPin, OUTPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(winSwitchPin, INPUT_PULLUP);

  digitalWrite(yellowLedPin, HIGH);
  digitalWrite(blueLedPin, HIGH);
  digitalWrite(pigPin, HIGH);
  delay(1000);
}

void loop() {
  unsigned long currentMillis = millis();
  switchOffThings();
  
  if (game.reset) {  
    blinkLED(currentMillis);
    countDownResetTimer(currentMillis);
  } else if (game.idle) {
    lightLED();
    waitForStartButton(currentMillis);
  } else if (game.gameloop) {
    countUpScoreTime(currentMillis);
    readFan();
    moveMotorAndPig();
    waitForWinSwitch();
  } else if (game.win) {
    // light up sun
    countDownWinTimer(currentMillis);
    Serial.print(game.timeScore);
  }
  
  delay(50);
}

void switchOffThings() {
  digitalWrite(yellowLedPin, LOW);
  digitalWrite(blueLedPin, LOW);
  digitalWrite(pigPin, LOW);
}

void countDownWinTimer(unsigned long cM) {
   if (cM - game.winDt >= game.winTimer) {
    game.winDt = cM;
    game.win = false;
    game.reset = true;
  }    
}

void waitForWinSwitch() {
  if (winSwitchPin == 1) {
    game.gameloop = false;
    game.win = true;
  }  
}

void countUpScoreTime(unsigned long cM) {
  game.timeScore = cM - game.timeScoreDt;
}

void waitForStartButton(unsigned long cM) {
  if (digitalRead(buttonPin) == 1) {
    game.idle = false;
    game.gameloop = true;
    game.timeScoreDt = cM;
  }  
}

void lightLED() {
   digitalWrite(blueLedPin, HIGH);
}

void countDownResetTimer(unsigned long cM) {
  if (cM - game.resetDt >= game.resetTimer) {
    game.resetDt = cM;
    game.reset = false;
    game.idle = true;
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
  if (debug) {
    Serial.print(fanVal);
    Serial.print(" ,");
    Serial.print(filterVal);
    Serial.println(" ,0,1023");
  }
}

void moveMotorAndPig() {
 if (filterVal >= motorThreshold) {
    digitalWrite(motorPin, HIGH);
    digitalWrite(pigPin, HIGH);
  } else {
    digitalWrite(motorPin, LOW);
    digitalWrite(pigPin, LOW);
  }
}
