#include <Arduino.h>

//for MIDI IN
#define MIDI_START 250
#define MIDI_STOP 252
#define MIDI_CONT 251
#define MIDI_CLOCK 248
//for MIDI OUT
#define DEFAULT_NOTE 60
#define STEP_LENGTH 8

//for midi in
int count = 0; // Zählvariable in einer Viertelnote werden 24 MIDI-Clock signale gesendet
byte speed = 2; //1=24ticks,2=12ticks,4=6ticks
//true disables midi, and writes on 9600 baud
bool debug = false;
// save button
byte setSlidePin = 11;
byte funcPin = 12;
byte noteUpDownPin = 13;
byte nextPrevPin = 10;

//buttons
bool nextPrevButtonPressed = false;
bool setSlideButtonPressed = false;
bool noteUpDownButtonPressed = false;
bool funcButtonPressed = false;

bool nextPrevButtonState = false;
bool setSlideButtonState = false;
bool noteUpDownButtonState = false;
bool funcButtonState = false;
//time to let choosen step blink without delay
unsigned int time;
unsigned int oldTime;

bool stopped = false;
bool gate[STEP_LENGTH];
byte notes[STEP_LENGTH];
byte ledPins[STEP_LENGTH] = {2,3,4,5,6,7,8,9};

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

void activeMenuBlink(){
  time = millis();
  if(time > oldTime + 250) {
    if(activeMenuLedState == true) {
      //if(gate[activeMenuStep] == false) {
        digitalWrite(ledPins[activeMenuStep],LOW);
        activeMenuLedState = false;
      //}
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
    if(activeMenuStep == STEP_LENGTH - 1) {
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
      activeMenuStep = STEP_LENGTH - 1;
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
    sendMidi(0,9,notes[activeStep],64);
  }
}

void checkButtons(){
  // read the state of the buttons
  nextPrevButtonState = digitalRead(nextPrevPin);
  setSlideButtonState = digitalRead(setSlidePin);
  noteUpDownButtonState = digitalRead(noteUpDownPin);
  funcButtonState = digitalRead(funcPin);

  //check noteUpDown
  if(noteUpDownButtonState == HIGH && noteUpDownButtonPressed == false) {
    if(funcButtonState == true) {
      //NoteDown
      if(debug) {
        Serial.println("NoteDown");
      }
      notes[activeMenuStep] = notes[activeMenuStep] - 1;
    }else {
      //NoteUp
      if(debug) {
        Serial.println("NoteUp");
      }
      notes[activeMenuStep] = notes[activeMenuStep] + 1;
    }
    noteUpDownButtonPressed = true;

  }
  if(noteUpDownButtonState == LOW) {
    noteUpDownButtonPressed = false;
  }


  if(setSlideButtonState == HIGH && setSlideButtonPressed == false) {
    if(funcButtonState == true) {
      //slide
      //have to work with note off, and a slide array, if slide is set, note off shouldnt be sent for next step
    } else {
      //set
      if(stopped) {
        sendMidi(0x0, 0x9, 0x3C, 0x40);
      }
      gate[activeMenuStep] = !gate[activeMenuStep];
      if(debug) {
        Serial.println(activeMenuStep);
        Serial.println(gate[activeStep]);
      }
    }

    setSlideButtonPressed = true;
  }
  if(setSlideButtonState ==LOW) {
    setSlideButtonPressed = false;
  }
  if(nextPrevButtonState == HIGH && nextPrevButtonPressed==false ) {
    if(funcButtonState == true) {
      //previous Step
      if(debug) {
        Serial.println("PRESS PREV");
      }
      prevStep();
    } else {
      //next Step
      if(debug) {
        Serial.println("PRESS NEXT");
      }
      nextStep();
    }
    nextPrevButtonPressed = true;
  }
  if(nextPrevButtonState == LOW) {
    nextPrevButtonPressed = false;
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

  for(int i=0;i<STEP_LENGTH;i++) {
    gate[i] = false;
    pinMode(ledPins[i],OUTPUT);
    notes[i] = DEFAULT_NOTE;
  }
  // initialize button pins
  pinMode(nextPrevPin, INPUT);
  pinMode(setSlidePin,INPUT);
  pinMode(noteUpDownPin, INPUT);
  pinMode(funcPin, INPUT);
}

void loop() {
  //blinkPin(activeStep, activeStep-1);
  checkButtons();
  activeMenuBlink();
  if (Serial.available()  > 0) {
    byte byte_read = Serial.read();

    if (byte_read == MIDI_START) {
      activeStep = 0;
      stopped = false;
    }

    if (byte_read == MIDI_STOP) {
      stopped = true;
      blinkPin(0, activeStep);
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
