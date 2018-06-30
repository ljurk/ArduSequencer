// ---------------------------------------------------------------------------
//
// basic.ino
//
// A MIDI sequencer example using a standard MIDI cable and a push button
// attached to pin 4.
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
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
byte setPin = 12;
byte btnPin = 13;
byte prevPin = 11;
byte stepSize = 8;
bool nextButtonPressed = false;
bool setButtonPressed = false;
bool prevButtonPressed = false;
bool nextButtonState = false;
bool setButtonState = false;
bool prevButtonState = false;

bool stopped = false;
bool gate[8];
byte ledPins[8] = {2,3,4,5,6,7,8,9};

byte activeStep= 0;
byte oldStep= 0;
bool activeLedState = false;
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
  //MIDI.sendNoteOn(42, 127, channel);
}

void blinkPin(byte pin, byte maxPin) {
  digitalWrite(ledPins[pin],HIGH);
  digitalWrite(ledPins[maxPin],LOW);
  for(int i = 0; i < stepSize; i++) {
    if(gate[i] == true) {
      if(pin == i) {
        if(activeLedState) {
            digitalWrite(ledPins[i],LOW);
            activeLedState = false;
        }else {
          digitalWrite(ledPins[i],HIGH);
          activeLedState = true;
        }
      }
      digitalWrite(ledPins[i],HIGH);
    } else {
      if(i != pin) {
        digitalWrite(ledPins[i],LOW);
      }
    }
  }
}

void nextStep() {
  if(stopped) {
    oldStep= activeStep;
    if(activeStep == stepSize - 1) {
      activeStep = 0;
    }else{
      activeStep++;
    }
    if(debug){
      sprintf(buffer,"active %d",activeStep);
      Serial.println(buffer);
    }
    blinkPin(activeStep,oldStep);
    if(gate[activeStep] && stopped) {
      if(debug){
        Serial.print(activeStep);
      }
      sendMidi(0x0, 0x9, 0x3C, 0x40);
    }
  }
}
void prevStep() {
  if(stopped) {
    oldStep= activeStep;
    if(activeStep == 0) {
      activeStep = 7;
    }else{
      activeStep--;
    }
    if(debug){
      sprintf(buffer,"active %d",activeStep);
      Serial.println(buffer);
    }
    blinkPin(activeStep,oldStep);
    if(gate[activeStep] && stopped) {
      if(debug){
        Serial.print(activeStep);
      }
      sendMidi(0x0, 0x9, 0x3C, 0x40);
    }
  }
}

void step() {
   if(activeStep == 0) {
     oldStep = 7;
   } else {
     oldStep = activeStep -1;
   }
    blinkPin(activeStep,oldStep);

  if(debug){
    sprintf(buffer,"current %d",activeStep);
    Serial.println(buffer);
  }
      if(gate[activeStep] == true) {
        if(debug){
          sprintf(buffer,"sendNote %d",activeStep);
          Serial.println(buffer);

        }
        //seq.setNote(0x0, 0x3C, 0x40);
        sendMidi(0x0, 0x9, 0x3C, 0x40);

      }
    //  transported[current]= true;

    if(activeStep == 7){
      activeStep=0;
    } else {
      activeStep++;
    }
}

void checkButtons(){
  // read the state of the buttons
  nextButtonState = digitalRead(btnPin);
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
    gate[activeStep] = !gate[activeStep];
    if(debug) {
      Serial.println(activeStep);
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
  pinMode(btnPin, INPUT);
  pinMode(setPin,INPUT);
  pinMode(prevPin, INPUT);
}

void loop() {
  //blinkPin(activeStep, activeStep-1);
  checkButtons();

  if (Serial.available()  > 0) {
    byte byte_read = Serial.read();

    if (byte_read == MIDI_START) {
      activeStep = 0;
      stopped = false;
    }

    if (byte_read == MIDI_STOP) {
      stopped = true;
      count = 0;
    }

    if (byte_read == MIDI_CONT) {
      stopped = false;
    }

    if (byte_read == MIDI_CLOCK && stopped == false) {
      count++;
      if (count == (24 / speed)) {
        step();
      }
    }
  }

  if (count > (24 / speed)) {
    count = 0;
  }
}
