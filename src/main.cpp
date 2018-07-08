#include <Arduino.h>

//for MIDI IN
#define MIDI_START 250
#define MIDI_STOP 252
#define MIDI_CONT 251
#define MIDI_CLOCK 248
//for MIDI OUT
#define MIDI_CHANNEL 0 //0-15 represents channels 1-16
#define NOTE_OFF 8
#define NOTE_ON 9
#define DEFAULT_VELOCITY 64
#define STEP_LENGTH 8
//buttons
#define SET_SLIDE_PIN  11
#define FUNC_PIN 12
#define  NOTE_UP_DOWN_PIN 13
#define NEXT_PREV_PIN 10
#define BLINK_TIME 150

//for midi in
int count = 0; // counter for midi ticks, 24 ticks are one quarter note
byte speedDivider = 1; //1=24ticks,2=12ticks,4=6ticks
//true disables midi, and writes debug messages on 9600 baud
bool debug = false;
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
byte DEFAULT_NOTE =  0;

bool stopped = true;
bool gate[STEP_LENGTH];
byte notes[STEP_LENGTH];
bool slide[STEP_LENGTH];
byte ledPins[STEP_LENGTH] = {9,8,7,6,5,4,3,2};

byte activeStep= 0;
byte oldStep= 0;
byte oldMenuStep= 0;
byte activeMenuStep=0;
bool activeMenuLedState = false;
char buffer[20];

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

void resetSequence(){
  for(byte i = 0; i < STEP_LENGTH;i++){
    gate[i] = false;
    slide[i] = false;
    notes[i] = DEFAULT_NOTE;
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
  if(time > oldTime + BLINK_TIME) {
    if(activeMenuLedState == true) {
        digitalWrite(ledPins[activeMenuStep],LOW);
        activeMenuLedState = false;
    }else {
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
      sprintf(buffer,"active %d",activeMenuStep);
      Serial.println(buffer);
    }
    if(gate[oldMenuStep]) {
      digitalWrite(ledPins[oldMenuStep],HIGH);
    }else{
      digitalWrite(ledPins[oldMenuStep],LOW);
    }
    if(gate[activeMenuStep] && stopped) {
      if(debug){
        Serial.print(activeMenuStep);
      }
      //sendMidi(MIDI_CHANNEL, NOTE_ON, notes[activeMenuStep], DEFAULT_VELOCITY);
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
    if(gate[activeMenuStep] && stopped) {
      if(debug){
        Serial.print(activeMenuStep);
      }
      //sendMidi(MIDI_CHANNEL, NOTE_ON, notes[activeMenuStep], DEFAULT_VELOCITY);
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
   if(gate[oldStep] && !slide[activeStep]) {
     sendMidi(MIDI_CHANNEL,NOTE_ON,notes[oldStep],0);
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
    sendMidi(MIDI_CHANNEL,NOTE_ON,notes[activeStep],DEFAULT_VELOCITY);
  }
}

void checkButtons(){
  // read the state of the buttons
  nextPrevButtonState = digitalRead(NEXT_PREV_PIN);
  setSlideButtonState = digitalRead(SET_SLIDE_PIN);
  noteUpDownButtonState = digitalRead(NOTE_UP_DOWN_PIN);
  funcButtonState = digitalRead(FUNC_PIN);

//if all buttons are pressed reset the seuqence
if(nextPrevButtonState && setSlideButtonState && noteUpDownButtonState && funcButtonPressed){
  resetSequence();
}
  //check noteUpDown
  if(noteUpDownButtonState == HIGH && noteUpDownButtonPressed == false) {
    if(funcButtonState == true) {
      //NoteDown
      if(debug) {
        Serial.println("NoteDown");
      }
      if(stopped && DEFAULT_NOTE != 0){
        DEFAULT_NOTE--;
      }
      notes[activeMenuStep] = notes[activeMenuStep] - 1;
    }else {
      //NoteUp
      if(debug) {
        Serial.println("NoteUp");
      }
      if(stopped){
        DEFAULT_NOTE++;
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
      slide[activeMenuStep] = !slide[activeMenuStep];
      //have to work with note off, and a slide array, if slide is set, note off shouldnt be sent for next step
    } else {
      //set
      if(stopped) {
        if(debug) {
          Serial.println("NOTE ON");
        }
        sendMidi(MIDI_CHANNEL, NOTE_ON, DEFAULT_NOTE, DEFAULT_VELOCITY);
      }
      gate[activeMenuStep] = !gate[activeMenuStep];
      notes[activeMenuStep] = DEFAULT_NOTE;
      if(debug) {
        Serial.println(activeMenuStep);
        Serial.println(gate[activeStep]);
      }
    }

    setSlideButtonPressed = true;
  }
   if(stopped && setSlideButtonState== LOW && setSlideButtonPressed == true) {
     if(debug) {
       Serial.println("NOTE OFFN");
     }
      //sendMidi(MIDI_CHANNEL, NOTE_OFF, notes[activeMenuStep], DEFAULT_VELOCITY);
      sendMidi(MIDI_CHANNEL, NOTE_ON, DEFAULT_NOTE, 0);
        setSlideButtonPressed = false;
  }
  //if(setSlideButtonState ==LOW) {

  //}
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

  time =millis();
  oldTime=time;
  if(debug) {
    //set baud rate for serial monitor
    Serial.begin(9600);
  }else{
    // set MIDI baud
    Serial.begin(31250);
  }

  //initialize arrays
  for(int i=0;i<STEP_LENGTH;i++) {
    gate[i] = false;
    pinMode(ledPins[i],OUTPUT);
    notes[i] = DEFAULT_NOTE;
    slide[i] = false;
  }

  // initialize button pins
  pinMode(NEXT_PREV_PIN, INPUT);
  pinMode(SET_SLIDE_PIN,INPUT);
  pinMode(NOTE_UP_DOWN_PIN, INPUT);
  pinMode(FUNC_PIN, INPUT);
}

void loop() {
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
      sendMidi(MIDI_CHANNEL,NOTE_ON,notes[0],0);
      activeStep=0;
      count = 0;
    }

    if (byte_read == MIDI_CONT) {
      stopped = false;
    }

    if (byte_read == MIDI_CLOCK && stopped == false) {
      count++;
      if (count == (24 / speedDivider)) {
        step();
        count = 0;
      }
    }
  }
}
