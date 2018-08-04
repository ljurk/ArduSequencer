#include <Arduino.h>

//for MIDI IN
#define MIDI_START 250
#define MIDI_STOP 252
#define MIDI_CONT 251
#define MIDI_CLOCK 248
//for MIDI OUT
#define MIDI_CHANNEL 10 //0-15 represents channels 1-16
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

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
int debounceDelay = 150;
//time to let choosen step blink without delay
unsigned int time;
unsigned int oldTime;
byte defaultNote =  0;

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
  for(byte i = 0; i < STEP_LENGTH;i++) {
    gate[i] = false;
    slide[i] = false;
    notes[i] = defaultNote;
    digitalWrite(ledPins[i], LOW);
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
    } else {
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
  } else {
    activeMenuStep++;
  }
  if(debug) {
    sprintf(buffer,"active %d",activeMenuStep);
    Serial.println(buffer);
  }
  if(gate[oldMenuStep]) {
    digitalWrite(ledPins[oldMenuStep],HIGH);
  } else {
    digitalWrite(ledPins[oldMenuStep],LOW);
  }
}

void prevStep() {
  oldMenuStep= activeMenuStep;
  if(activeMenuStep == 0) {
    activeMenuStep = STEP_LENGTH - 1;
  } else {
    activeMenuStep--;
  }
  if(debug) {
    sprintf(buffer,"active %d",activeMenuStep);
    Serial.println(buffer);
  }
  if(gate[oldMenuStep]) {
    digitalWrite(ledPins[oldMenuStep],HIGH);
  } else {
    digitalWrite(ledPins[oldMenuStep],LOW);
  }
}

void step() {
  if(activeStep == STEP_LENGTH - 1) {
    activeStep=0;
  } else {
    activeStep++;
  }
  if(activeStep == 0) {
    oldStep = STEP_LENGTH - 1;
  } else {
    oldStep = activeStep -1;
 }
 if(gate[oldStep] && !slide[activeStep]) {
   sendMidi(MIDI_CHANNEL,NOTE_ON,notes[oldStep],0);
 }
 if(debug) {
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
  if(nextPrevButtonState && setSlideButtonState && noteUpDownButtonState && funcButtonPressed) {
    resetSequence();
    lastDebounceTime =millis();
  }
  //check noteUpDown
  if(noteUpDownButtonState == HIGH && noteUpDownButtonPressed == false) {
    lastDebounceTime = millis();
    if(funcButtonState == true) {
      //NoteDown
      if(debug) {
        Serial.println("NoteDown");
      }
      if(stopped && defaultNote != 0) {
        defaultNote--;
      }
      notes[activeMenuStep] = notes[activeMenuStep] - 1;
    } else {
      //NoteUp
      if(debug) {
        Serial.println("NoteUp");
      }
      if(stopped) {
        defaultNote++;
      }
      notes[activeMenuStep] = notes[activeMenuStep] + 1;
    }
    noteUpDownButtonPressed = true;
  }
  if(noteUpDownButtonState == LOW) {
    noteUpDownButtonPressed = false;
  }
  //check setSlideButton
  if(setSlideButtonState == HIGH && setSlideButtonPressed == false) {
    lastDebounceTime = millis();
    if(funcButtonState == true) {
      //set slide for activeM
      slide[activeMenuStep] = !slide[activeMenuStep];
    }else{
      //set
      if(stopped) {
        if(debug) {
          Serial.println("NOTE ON");
        }
        sendMidi(MIDI_CHANNEL, NOTE_ON, defaultNote, DEFAULT_VELOCITY);
      }
      gate[activeMenuStep] = !gate[activeMenuStep];
      digitalWrite(ledPins[activeMenuStep],gate[activeMenuStep]);
      notes[activeMenuStep] = defaultNote;
      if(debug) {
        Serial.println(activeMenuStep);
        Serial.println(gate[activeMenuStep]);
      }
    }
    setSlideButtonPressed = true;
  }
  if(stopped && setSlideButtonState== LOW && setSlideButtonPressed == true) {
    lastDebounceTime = millis();
    if(debug) {
      Serial.println("NOTE OFFN");
    }
    sendMidi(MIDI_CHANNEL, NOTE_ON, defaultNote, 0);
    setSlideButtonPressed = false;
  }
  if(setSlideButtonState ==LOW) {
    setSlideButtonPressed = false;
  }
  if(nextPrevButtonState == HIGH && nextPrevButtonPressed==false ) {
    lastDebounceTime = millis();
    if(funcButtonState == true) {
      //previous Step
      if(debug) {
        Serial.println("PRESS PREV");
      }
      prevStep();
    }else{
      //next Step
      if(debug) {
        if(debug){
          sprintf(buffer,"next %ld",lastDebounceTime);
          Serial.println(buffer);
        }
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
  } else {
    // set MIDI baud
    Serial.begin(31250);
  }

  //initialize arrays
  for(int i=0;i<STEP_LENGTH;i++) {
    gate[i] = false;
    pinMode(ledPins[i],OUTPUT);
    notes[i] = defaultNote;
    slide[i] = false;
  }

  // initialize button pins
  pinMode(NEXT_PREV_PIN, INPUT);
  pinMode(SET_SLIDE_PIN,INPUT);
  pinMode(NOTE_UP_DOWN_PIN, INPUT);
  pinMode(FUNC_PIN, INPUT);
  lastDebounceTime = millis();
}

void loop() {
  if(millis() - debounceDelay > lastDebounceTime) {
    checkButtons();
  }
  activeMenuBlink();
  if(Serial.available()  > 0) {
    byte byte_read = Serial.read();
    switch(byte_read) {
      case MIDI_START:
        activeStep = 0;
        stopped = false;
        break;
      case MIDI_STOP:
        stopped = true;
        blinkPin(0, activeStep);
        sendMidi(MIDI_CHANNEL,NOTE_ON,notes[0],0);
        activeStep=0;
        count = 0;
        break;
      case MIDI_CONT:
        stopped = false;
        break;
      case MIDI_CLOCK:
        if(!stopped) {
          count++;
          if(count == (24 / speedDivider)) {
            step();
            count = 0;
          }
        }
        break;
      default:
        break;

    }
  }
}
