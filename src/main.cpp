/*// ---------------------------------------------------------------------------
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
#include "FifteenStep.h"

// sequencer init
FifteenStep seq = FifteenStep();

// save button state
byte setPin = 12;
byte btnPin = 13;
byte stepSize = 8;
int button_last = 0;
bool nextButtonPressed = false;
bool setButtonPressed = false;
bool stopped = false;
bool gate[8];
bool transported[8];
int activeStep= 0;

void blinkPin(byte pin, byte maxPin) {
  digitalWrite(pin+2,HIGH);
  digitalWrite(maxPin+2,LOW);
  Serial.println(pin);
  for(int i = 0; i < stepSize; i++) {
    if(gate[i] == true) {
      digitalWrite(i+2,HIGH);
    } else {
      if(i != activeStep) {
        digitalWrite(i+2,LOW);
      }
    }
  }
}

void nextStep() {
  int old = activeStep;
  if(activeStep < stepSize) {
    activeStep++;
  }else{
    activeStep = 0;
  }
  blinkPin(activeStep,old);
}
void step(int current, int last) {
  // blink on even steps
  //blinkPin(current,last);
  nextStep();
  if(transported[current] == false) {
    seq.setNote(0x0, 0x3C, 0x40);
    transported[current]= true;
  }
}

void midi(byte channel, byte command, byte arg1, byte arg2) {
  if(command < 128) {
    // shift over command
    command <<= 4;
    // add channel to the command
    command |= channel;
  }
  // send MIDI data
  //Serial.write(command);
  //Serial.write(arg1);
  //Serial.write(arg2);
}

void setup() {
  // set MIDI baud
//  Serial.begin(9600);
Serial.begin(31250);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6 ,OUTPUT);
  pinMode(7 ,OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

for(int i=0;i<stepSize;i++) {
  gate[i] = false;
  transported[i] = true;
}
  // initialize digital pin 4 as an input for a button
  pinMode(btnPin, INPUT);
  pinMode(setPin,INPUT);

  // start sequencer and set callbacks
  seq.begin(160,stepSize);
  seq.setMidiHandler(midi);
  seq.setStepHandler(step);

}

void loop() {
  // read the state of the button
  int nextbutton = digitalRead(btnPin);
  int setbutton = digitalRead(setPin);
  // check for button press or release and
  // send note on or off to seqencer if needed
  if(nextbutton == HIGH && setbutton == HIGH) {
    // button pressed. play middle C preview now
  //  midi(0x0, 0x9, 0x3C, 0x40);
    // store note in sequence
    //seq.setNote(0x0, 0x3C, 0x40);
    if(stopped == false) {
      seq.stop();
      stopped = true;
    } else {
      seq.start();
      stopped = false;
    }

  }
  if(nextbutton == LOW && setbutton == HIGH && setButtonPressed == false) {
    gate[activeStep] = !gate[activeStep];
    transported[activeStep] = false;
    Serial.println(activeStep);
    Serial.println(gate[activeStep]);
    setButtonPressed = true;
    // button released. send middle C note off preview now
  //  midi(0x0, 0x8, 0x3C, 0x0);
    // store note off in sequence
  //  seq.setNote(0x0, 0x3C, 0x0);

}
if(setbutton ==LOW) {
  setButtonPressed = false;
}
if(nextbutton == HIGH && setbutton == LOW && nextButtonPressed==false ) {
  nextButtonPressed = true;
  Serial.println("PRESS NEXT");
  nextStep();
}
if(nextbutton == LOW) {
  nextButtonPressed = false;
}

  // save button state

  // this is needed to keep the sequencer
  // running. there are other methods for
  // start, stop, and pausing the steps
  seq.run();

}
*/
