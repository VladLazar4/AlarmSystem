#include "pitches.h"
#include <Keypad.h>

const int buzzerPin = 2;
const int motionSensorPin = 10;
const int redPin = 13;
const int greenPin = 12;
const int bluePin = 11;

const int ROW_NUM = 4;
const int COLUMN_NUM = 3;

char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

byte pin_rows[ROW_NUM] = { 9, 8, 7, 6 };
byte pin_column[COLUMN_NUM] = { 5, 4, 3 };

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

int motionDetected = LOW;

String inputPin = "";
String pin = "1234";
bool firstPressStar = false;

int state = 2;
int prevState = -1;

void setup() {
  keypad.addEventListener(keypadEvent);
  pinMode(buzzerPin, OUTPUT);
  pinMode(motionSensorPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  listenBT();

  if(state!=prevState){
    prevState=state;
    Serial.print(state);
  }
  
  switch (state) {
    case 0:  //armat
      keypad.getKey();
      setColor(255, 0, 0);
      motionDetected = digitalRead(motionSensorPin);
      if (motionDetected == HIGH) {
        state = 3;
      }
      break;
    case 1:  //armare
      keypad.getKey();
      setColor(255,165,0);
      arming();
      break;
    case 2:  //dezarmat
      keypad.getKey();
      setColor(0, 255, 0);
      break;
    case 3:  //trigered
      keypad.getKey();
      setColor(0, 0, 255);
      triggered();
      break;
    case 4:  //allert
      keypad.getKey();
      startAlarm();
      break;
  }
}

void keypadEvent(KeypadEvent eKey) {
  switch (keypad.getState()) {
    case PRESSED:
      switch (eKey) {
        case '#': checkPin(); break;
        case '*': changePin(); break;
        default: inputPin += eKey;
      }
  }
}

void checkPin() {
  if (inputPin == pin) {
    switch (state) {
      case 0: state = 2; break;
      case 2: state = 1; break;
      case 3: state = 2; break;
      case 4: state = 2; break;
      default: state = 1;
    }
  } else {
    state = 4;
  }
  inputPin = "";
}

void changePin() {
  if (state == 2) {
    if (firstPressStar == false) {
      firstPressStar = true;
    } else {
      if(inputPin.length()>=4){
        pin = inputPin;
        Serial.print(pin);
        colorSplash(0,255,0);
      } else{
        colorSplash(255,0,0);
      }
      firstPressStar = false;
    }
  }
  inputPin = "";
}

void colorSplash(int redValue, int greenValue, int blueValue){
  setColor(0,0,0);
  delay(100);
  setColor(redValue,greenValue,blueValue);
  delay(200);
  setColor(0,0,0);
  delay(100);
  setColor(redValue,greenValue,blueValue);
  delay(200);
}

void arming() {
  unsigned long startTime = millis();
  while (state == 1 && millis() - startTime <= 5000) {
    if ((millis() - startTime) % 1000 == 0) {
      tone(buzzerPin, 1000);
      delay(100);
      stopSound();
    }
  }
  stopSound();
  state = 0;
}

void triggered() {
  unsigned long startTime = millis();
  while (state == 3 && millis() - startTime <= 5000) {
    keypad.getKey();
    if ((millis() - startTime) % 1000 == 0) {
      tone(buzzerPin, 1000);
      delay(100);
      stopSound();
    }
  }
  stopSound();
  if (state == 3)
    state = 4;
}

void startAlarm() {
  int alarm[] = {
    NOTE_C6, NOTE_E6, NOTE_G6,
    0,
    NOTE_C6, NOTE_E6, NOTE_G6,
    0,
    NOTE_C6, NOTE_E6, NOTE_G6,
    0
  };

  int noteDurations[] = {
    100, 100, 100,
    100,
    100, 100, 100,
    100,
    100, 100, 100,
    100
  };

  while (state == 4) {
    keypad.getKey();
    listenBT();
    for (int i = 0; i < 12; i++) {
      keypad.getKey();
      int noteDuration = noteDurations[i];
      tone(buzzerPin, alarm[i], noteDuration);

      if (alarm[i] == 0) {
        setColor(0, 0, 0);
      } else {
        setColor(255, 0, 0);
      }

      delay(noteDuration * 1.20);
      noTone(buzzerPin);
    }
  }
}

void listenBT(){
  if(Serial.available()>0){
    char aux = Serial.read();
    if(aux=='*'){
      for(int i=0;i<4;i++){
        Serial.available();
        inputPin[i] = Serial.read();
      }
      Serial.println(inputPin);
    } else{
      if(aux=='1')
        state = 1;
      if(aux=='2')
        state = 2;
    }
  }
}

void stopSound() {
  noTone(buzzerPin);
}

void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}

// void sendByteBT(byte x){
//   bt.write(x);
//   Serial.println(x);
// }


// void waitInputPin() {
//   setColor(0, 0, 255);
//   char key = keypad.getKey();
//   bool pinEntered = false;
//   bool validPin = false;
//   unsigned long startTime = millis();
//   while (pinEntered == false && millis() - startTime <= 5000) {
//     if (key) {
//       if (key == '#') {
//         if (inputPin == pin) {
//           Serial.println("Access granted");
//           state = 2;
//           validPin = true;
//         } else {
//           Serial.println("Access declined");
//           validPin = false;
//         }
//         inputPin = "";
//       } else
//         inputPin += key;
//     }
//   }
//   if (validPin == false)
//     state = 3;
// }