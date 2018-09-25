/*<><><><><><><><><<<code by Lukas Jurk>>><><><><><><><><>*/
/*<><><><><><><><><<<version 0.23>>><><><><><><><><>*/
/*<><><><><><><><><<<303>>><><><><><><><><>*/
/*#include <Arduino.h>
#include <Encoder.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#include "..\lib\seq.hpp"
#include "..\lib\midi.hpp"

#define STEP_LENGTH 8
//for midi in

//true disables midi, and writes debug messages on 9600 baud
bool debugON = false;

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

byte ledPins[STEP_LENGTH] = {9,8,7,6,5,4,3,2};

bool activeMenuLedState = true;
char buffer[20];
sequencer seq = sequencer(debugON);
void startingAnimation(){
  if(debugON) {
    Serial.print("starting...");
  }
  for(int i = 0; i  <STEP_LENGTH; i++) {
    digitalWrite(ledPins[i],HIGH);
    delay(500);
  }
  for(int h = 0; h  <5; h++) {
    for(int i = 0; i  <STEP_LENGTH; i++) {
      digitalWrite(ledPins[i],HIGH);
    }
    delay(200);
    for(int i = 0; i  <STEP_LENGTH; i++) {
      digitalWrite(ledPins[i],LOW);
    }
    delay(200);
  }
}
void pressNext() {
  if(debugON) {
    sprintf(buffer,"Active %x",seq.getActiveMenuStep());
    Serial.println(buffer);
    sprintf(buffer,"Old %x",seq.getOldMenuStep());
    Serial.println(buffer);
  }
  if(seq.getGate(seq.getOldMenuStep())) {
    digitalWrite(ledPins[seq.getOldMenuStep()],HIGH);
  } else {
    digitalWrite(ledPins[seq.getOldMenuStep()],LOW);
  }
}
void pressPrev(){
  if(debugON) {
    sprintf(buffer,"Active %x",seq.getActiveMenuStep());
    Serial.println(buffer);
    sprintf(buffer,"Old %x",seq.getOldMenuStep());
    Serial.println(buffer);
  }
  if(seq.getGate(seq.getOldMenuStep()))  {
    digitalWrite(ledPins[seq.getOldMenuStep()],HIGH);
  } else {
    digitalWrite(ledPins[seq.getOldMenuStep()],LOW);
  }
}
void showSequence() {
  for(int i = 0; i < STEP_LENGTH; i++) {
   if(i != seq.getActiveMenuStep()) {
      if(seq.getGate(i)) {
        if(i == seq.getActiveStep()) {
            digitalWrite(ledPins[i], LOW);
          } else {
            digitalWrite(ledPins[i],HIGH);
          }
      } else {
        if(i == seq.getActiveStep()) {
            digitalWrite(ledPins[i], HIGH);
          } else {
            digitalWrite(ledPins[i],LOW);
          }
      }
    }
  }
}
void blinkPin(byte blink, byte unblink) {
  digitalWrite(ledPins[blink],HIGH);
  if(seq.getGate(unblink) == false) {
    digitalWrite(ledPins[unblink],LOW);
  }
}

void activeMenuBlink(){
  time = millis();
  if(time > oldTime + BLINK_TIME) {
    if(debugON) {
      seq.clock();
    }
    if(debugON) {
      //  Serial.println(seq.getActiveMenuStep());
    }

    if(activeMenuLedState == true) {
        digitalWrite(ledPins[seq.getActiveMenuStep()],LOW);
        activeMenuLedState = false;
    } else {
      digitalWrite(ledPins[seq.getActiveMenuStep()],HIGH);
      activeMenuLedState = true;
    }
    oldTime = time;
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
    seq.resetSequence();
    lastDebounceTime =millis();
  }
  //check noteUpDown
  if(noteUpDownButtonState == HIGH && noteUpDownButtonPressed == false) {
    lastDebounceTime = millis();
    if(funcButtonState == true) {
      //NoteDown
      if(debugON) {
        Serial.println("NoteDown");
      }
      if(seq.getStopped()) {
        seq.defaultNoteDown();
      }
      seq.noteDown();
    } else {
      //NoteUp
      if(debugON) {
        Serial.println("NoteUp");
      }
      if(seq.getStopped()) {
        seq.defaultNoteUp();
      }
      seq.noteUp();
    }
    noteUpDownButtonPressed = true;
  }
  if(noteUpDownButtonState == LOW) {
    noteUpDownButtonPressed = false;
  }
  //check setSlideButton
  if(setSlideButtonState == HIGH && setSlideButtonPressed == false) {
    lastDebounceTime = millis();
    if(funcButtonState == true && seq.getSlideActive() == true) {
      //set slide for activeMenuStep
      seq.setSlide();
      //slide[activeMenuStep] = !slide[activeMenuStep];
    }else{
      //set
      if(seq.getStopped()) {
        if(debugON) {
          Serial.println("NOTE ON");
        }
        sendMidi(MIDI_CHANNEL, NOTE_ON, seq.getDefaultNote(), DEFAULT_VELOCITY);
        showSequence();
      }
      showSequence();
      seq.setGate();
      digitalWrite(ledPins[seq.getActiveMenuStep()],seq.getGate(seq.getActiveMenuStep()));
      seq.setNote();
    }
    setSlideButtonPressed = true;
  }
  if(seq.getStopped() && setSlideButtonState == LOW && setSlideButtonPressed == true) {
    lastDebounceTime = millis();
    if(debugON) {
      Serial.println("NOTE OFFN");
    }
    sendMidi(MIDI_CHANNEL, NOTE_ON, seq.getDefaultNote(), 0);
    setSlideButtonPressed = false;
  }
  if(setSlideButtonState == LOW) {
    setSlideButtonPressed = false;
  }
  //check nextPrev
  if(nextPrevButtonState == HIGH && nextPrevButtonPressed == false ) {
    lastDebounceTime = millis();
    if(funcButtonState == true) {
      //previous Step
      if(debugON) {
        Serial.println("PRESS PREV");
      }
      seq.prevStep();
      pressPrev();
    }else{
      //next Step
      if(debugON) {
        if(debugON) {
          sprintf(buffer,"next %ld",lastDebounceTime);
          Serial.println(buffer);
        }
      }
      seq.nextStep();
      pressNext();
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
  if(debugON) {
    //set baud rate for serial monitor
    Serial.begin(9600);
  } else {
    // set MIDI baud
    Serial.begin(31250);
  }
  for(int i = 0; i  <STEP_LENGTH; i++) {
    pinMode(ledPins[i],OUTPUT);

  }
  startingAnimation();

  // initialize button pins
  pinMode(NEXT_PREV_PIN, INPUT);
  pinMode(SET_SLIDE_PIN,INPUT);
  pinMode(NOTE_UP_DOWN_PIN, INPUT);
  pinMode(FUNC_PIN, INPUT);
  lastDebounceTime = millis();
  if(debugON){
    seq.start();
  }

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
          seq.start();
        break;
      case MIDI_STOP:
        seq.stop();
        blinkPin(0, seq.getActiveStep());
        break;
      case MIDI_CONT:
        seq.cont();
        break;
      case MIDI_CLOCK:
          showSequence();
          seq.clock();
        break;
      default:
        break;
    }
  }
}
*/




#include <Encoder.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "../lib/display.hpp"
#include "..\lib\seq.hpp"
#include "..\lib\midi.hpp"
// OLED display TWI address
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(-1);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define STEP_LENGTH 8
const int buttonPin = 4;
Encoder myEnc(2, 3);
int buttonState = 0;
bool buttonPressed = false;
long oldPosition  = -999;
long newPosition =  -999;
String seqString ="00000000 00000000";
String cursorString ="00000000 00000000";
byte mode = 0;


sequencer seq = sequencer();

void updateCursor() {
  /*Serial.println(actualStep);
  display.clearDisplay();
  updateSequence();
  updateValues();*/
  /*  byte adder = 0;

  if(actualStep >= (STEP_LENGTH / 2)) {
    adder = 1;
  } else {
    adder = 0;
  }
  Serial.println(actualStep);
  Serial.println(adder);*/
  if(mode == 0 ) {
    for(int i = 0; i < STEP_LENGTH ;i++) {
      if(i == STEP_LENGTH) {
        cursorString[i] = ' ';
      }
      if(i == seq.getActiveMenuStep()) {
          cursorString[i/*+ adder*/] ='^';
        } else {
          cursorString[i/* + adder*/] = ' ';
        }

    }

    display.setCursor(0,15);
    display.print(cursorString);
}

void updateValues(){
  display.setTextSize(1);
  display.setCursor(0,40);
  if(mode == 1) {
    display.setTextColor(BLACK);
    display.fillRect(0,48, 64, 16, WHITE);
  } else {
    display.setTextColor(WHITE);
  }
  String outNote = "NOTE ";
  outNote += String(note);
  String outVelo= " VELO ";
  outVelo += String(velocity[seq.getActiveMenuStep()]);
  display.setCursor(0,48);
  display.print(outNote);
  if(mode == 2) {
    display.setTextColor(BLACK);
    display.fillRect(64,48, 64, 16, WHITE);
  } else {
    display.setTextColor(WHITE);
  }
  display.setCursor(64,48);
  display.print(outVelo);
  display.setTextColor(WHITE);
  //display.setTextSize(2);
}

void updateSequence(){
  byte adder = 0;
  if(actualStep >= (STEP_LENGTH / 2)) {
    adder = 1;
  } else {
    adder = 0;
  }
  for(int i = 0; i  <STEP_LENGTH + 1; i++) {
    if(i >= (STEP_LENGTH / 2)) {
      adder = 1;
    } else {
      adder = 0;
    }
    if(i == STEP_LENGTH / 2) {
      seqString[i] = ' ';
    }
      if(gate[i] == true) {
        seqString[i + adder ] = 'X';
      } else {
        seqString[i + adder] = '-';
      }


  }
  //updateSequence();
  //updateValues(false);
  //display.clearDisplay();
  display.setCursor(0,0);
  display.print(seqString);
  //display.display();
}

void updateDisplay(){
  display.clearDisplay();
  updateValues();
  updateCursor();
  updateSequence();
  display.display();
}

void checkInputs(){
  newPosition = myEnc.read();
  buttonState = digitalRead(buttonPin);

  if (newPosition != oldPosition) {
    if(newPosition > oldPosition + 1) {
      //turn right
      if (buttonState == HIGH){
        mode++;
        if(mode == 3) {
          mode = 0;
        }
      } else{
        if(mode == 1) { //velo
          seq.noteUp();
          //updateValues();
          /*if(note == 127) {
            note = 60;
          } else {
              note ++;
          }*/
        }
        if(mode == 2) { //velo
          seq.setVelocityUp();
        }
        if (mode == 0){
          //turn right
          //actualStep ++;
          seq.nextStep();
          //updateCursor();
          /*if (actualStep == STEP_LENGTH + 1) {
            actualStep = 0;
          }*/
        }
      }

    } else if(newPosition < oldPosition - 1){
        //turn right
        if (buttonState == HIGH){
          mode--;
          if(mode == 255) {
            mode = 2;
          }
        } else{
          if(mode == 1) { //velo
            seq.noteDown();
            //updateValues();
            /*if(note == 60) {
              note = 127;
            } else {
                note --;
            }*/
          }
          if(mode == 2) { //velo
            seq.setVelocityDown();
          }
          if (mode == 0){
            //turn right
            seq.prevStep();
            //updateCursor();
            /*actualStep--;
            if (actualStep == 255) {
              actualStep = STEP_LENGTH;
            }*/
          }
        }
    }
    oldPosition = newPosition;

    /*display.clearDisplay();
    Serial.println(newPosition);
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(27,30);

    String myString = String(newPosition);
    display.print(myString);*/
    updateDisplay();
  }
  if (buttonState == HIGH && buttonPressed == false) {
    seq.setGate();
    seq.setNote();
    //updateSequence();
    updateDisplay();
    buttonPressed = true;
  } else if(buttonState == LOW) {
    buttonPressed = false;
  }
}
void setup() {
  // initialize and clear display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  // display a pixel in each corner of the screen
  display.drawPixel(0, 0, WHITE);
  display.drawPixel(127, 0, WHITE);
  display.drawPixel(0, 63, WHITE);
  display.drawPixel(127, 63, WHITE);

  // display a line of text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15,0);
  display.print("SEQ v3.0.3");
  display.setCursor(0,15);
  display.print("<§~#%/}{\%#~§>");

  display.drawRect(0,0, 128, 64, WHITE);
  display.drawLine(0,30, 128, 30, WHITE);
  display.fillRect(0,48, 128, 16, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(0,48);
  display.print("303030303");
//display.setTextSize(2);

  // update display with all of the above graphics
  display.display();
  pinMode(buttonPin,INPUT);
  Serial.begin(9600);
  for(int i = 0; i  <STEP_LENGTH; i++) {
    gate[i] = false;
  }
  delay(5000);
}

void loop() {
 // put your main code here, to run repeatedly:
 checkInputs();
 if(Serial.available()  > 0) {
   byte byte_read = Serial.read();
   switch(byte_read) {
     case MIDI_START:
         seq.start();
       break;
     case MIDI_STOP:
       seq.stop();
       //blinkPin(0, seq.getActiveStep());
       break;
     case MIDI_CONT:
       seq.cont();
       break;
     case MIDI_CLOCK:
         //showSequence();
         seq.clock();
       break;
     default:
       break;
   }
 }
  display.display();
}
