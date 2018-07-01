#include <Arduino.h>

#define MIDI_START 250
#define MIDI_STOP 252
#define MIDI_CONT 251
#define MIDI_CLOCK 248
//for midi in
int count = 0; // Zählvariable in einer Viertelnote werden 24 MIDI-Clock signale gesendet
byte speed = 2; //1=24ticks,2=12ticks,4=6ticks
//true disables midi, and writes on 9600 baud
bool debug = false;
// save button
byte setPin = 11;
byte nextPin = 12;
//byte prevPin = 11;
byte noteUpPin = 13;
byte prevPin = 10;
byte stepSize = 8;
//buttons
bool nextButtonPressed = false;
bool setButtonPressed = false;
bool prevButtonPressed = false;
bool nextButtonState = false;
bool setButtonState = false;
bool prevButtonState = false;
//time to let choosen step blink without delay
unsigned int time;
unsigned int oldTime;

bool stopped = false;
bool gate[8];
byte ledPins[8] = {2,3,4,5,6,7,8,9};

byte activeStep= 0;
byte oldStep= 0;
byte oldMenuStep= 0;
byte activeMenuStep=0;
bool activeMenuLedState = false;
char   buffer[20];

void sendMidi(byte channel, byte command, byte arg1, byte arg2) {
  if(command < 128) {
    // shift over command
    command <<= 4;
    // add channel to the command
    command |= channel;
  }
  // send MIDI data
  if(!debug) {
    Serial.write(command);
    Serial.write(arg1);
    Serial.write(arg2);
  } else {
    Serial.println("MIDI");
    Serial.println(command);
    Serial.println(arg1);
    Serial.println(arg2);
  }
}

void blinkPin(byte blink, byte unblink) {
  digitalWrite(ledPins[blink],HIGH);

  if(gate[unblink] == false) {
    digitalWrite(ledPins[unblink],LOW);
  }

}

void stepLed(){
  for(int i = 0; i < stepSize; i++) {
    if(gate[i] == true) {
        digitalWrite(ledPins[i],HIGH);

    } else {
      if (i != activeStep || i != activeMenuStep){
        digitalWrite(ledPins[i],LOW);
      }
    }
  }
}

void activeMenuBlink(){
  time = millis();
  if(time > oldTime + 250) {
    if(activeMenuLedState == true) {
      if(gate[activeMenuStep] == false) {
        digitalWrite(ledPins[activeMenuStep],LOW);
        activeMenuLedState = false;
      }
      if(debug){
        sprintf(buffer,"menuOn %d",activeMenuStep);
        Serial.println(buffer);
      }
    }else {
      if(debug){
        sprintf(buffer,"menuOff %d",activeMenuStep);
        Serial.println(buffer);
      }
      digitalWrite(ledPins[activeMenuStep],HIGH);
      activeMenuLedState = true;
    }
    oldTime = time;
  }

}

void nextStep() {
    oldMenuStep= activeMenuStep;
    if(activeMenuStep == stepSize - 1) {
      activeMenuStep = 0;
    }else{
      activeMenuStep++;
    }
    if(debug){
      sprintf(buffer,"active %d",activeStep);
      Serial.println(buffer);
    }
    //blinkPin(activeMenuStep,oldMenuStep);
    if(gate[oldMenuStep]) {
      digitalWrite(ledPins[oldMenuStep],HIGH);
    }else{
      digitalWrite(ledPins[oldMenuStep],LOW);
    }
    if(gate[activeMenuStep] && stopped) {
      if(debug){
        Serial.print(activeMenuStep);
      }
      sendMidi(0x0, 0x9, 0x3C, 0x40);
    }
  }

void prevStep() {
    oldMenuStep= activeMenuStep;
    if(activeMenuStep == 0) {
      activeMenuStep = 7;
    }else{
      activeMenuStep--;
    }
    if(debug){
      sprintf(buffer,"active %d",activeMenuStep);
      Serial.println(buffer);
    }
    if(gate[oldMenuStep]) {
      digitalWrite(ledPins[oldMenuStep],HIGH);
    }else{
      digitalWrite(ledPins[oldMenuStep],LOW);
    }
    //blinkPin(activeMenuStep,oldMenuStep);
    if(gate[activeMenuStep] && stopped) {
      if(debug){
        Serial.print(activeMenuStep);
      }
      sendMidi(0x0, 0x9, 0x3C, 0x40);
    }
}

void step() {


  if(activeStep ==7){
    activeStep=0;
  } else {
    activeStep++;
  }
   if(activeStep == 0) {
     oldStep = 7;
   } else {
     oldStep = activeStep -1;
   }
  if(debug){
    sprintf(buffer,"current %d",activeStep);
    Serial.println(buffer);
  }
  blinkPin(activeStep,oldStep);
  if(gate[activeStep] == true) {
    if(debug){
      sprintf(buffer,"sendNote %d",activeStep);
      Serial.println(buffer);
    }
    //seq.setNote(0x0, 0x3C, 0x40);
    //sendMidi(0x0, 0x9, 0x3C, 0x40);
    sendMidi(0,9,60,64);

  }
}

void checkButtons(){
  // read the state of the buttons
  nextButtonState = digitalRead(nextPin);
  setButtonState = digitalRead(setPin);
  prevButtonState = digitalRead(prevPin);

  if(prevButtonState == HIGH && prevButtonPressed == false) {
    prevButtonPressed = true;
    if(debug) {
      Serial.println("PRESS PREV");
    }
    prevStep();
  }
  if(prevButtonState == LOW) {
    prevButtonPressed = false;
  }
  if(nextButtonState == LOW && setButtonState == HIGH && setButtonPressed == false) {
    if(stopped) {
      sendMidi(0x0, 0x9, 0x3C, 0x40);
    }
    gate[activeMenuStep] = !gate[activeMenuStep];
    if(debug) {
      Serial.println(activeMenuStep);
      Serial.println(gate[activeStep]);
    }
    setButtonPressed = true;
  }
  if(setButtonState ==LOW) {
    setButtonPressed = false;
  }
  if(nextButtonState == HIGH && setButtonState == LOW && nextButtonPressed==false ) {
    nextButtonPressed = true;
    if(debug) {
      Serial.println("PRESS NEXT");
    }
    nextStep();
  }
  if(nextButtonState == LOW) {
    nextButtonPressed = false;
  }
}

void setup() {
  // set MIDI baud
  time =millis();
  oldTime=time;
  if(debug) {
    Serial.begin(9600);
  }else{
    Serial.begin(31250);
  }

  for(int i=0;i<stepSize;i++) {
    gate[i] = false;
    pinMode(ledPins[i],OUTPUT);
  }
  // initialize button pins
  pinMode(nextPin, INPUT);
  pinMode(setPin,INPUT);
  pinMode(prevPin, INPUT);
  gate[0] = true;
  gate[4] = true;
}

void loop() {
  //blinkPin(activeStep, activeStep-1);
  checkButtons();
  //stepLed(); to slow
  activeMenuBlink();
  if (Serial.available()  > 0) {
    byte byte_read = Serial.read();

    if (byte_read == MIDI_START) {
      activeStep = 0;
      stopped = false;
    }

    if (byte_read == MIDI_STOP) {
      stopped = true;
      activeStep=0;
      count = 0;
    }

    if (byte_read == MIDI_CONT) {
      stopped = false;
    }

    if (byte_read == MIDI_CLOCK && stopped == false) {
      count++;
      if (count == (24 / speed)) {
        step();
        count = 0;
      }
    }
  }

/*  if (count > (24 / speed)) {
    count = 0;
  }*/
}
