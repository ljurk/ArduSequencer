#include <Encoder.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "..\lib\display.hpp"
#include "..\lib\seq.hpp"
#include "..\lib\midi.hpp"
// OLED display TWI address
#define OLED_ADDR   0x3C
Adafruit_SSD1306 display(-1);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define STEP_LENGTH 8
bool debugON = false;

const int buttonPin = 10;
Encoder myEnc(2,3);
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
int debounceDelay = 300;
int buttonState = 0;
bool buttonPressed = false;
long oldPosition  = -999;
long newPosition =  -999;
String seqString ="00000000";
String cursorString ="00000000";
byte mode = 0;


sequencer seq = sequencer();

void updateCursor() {
  display.setTextSize(2);
  if(mode == 0 ) {
    for(int i = 0; i < STEP_LENGTH ;i++) {
      if(i == seq.getActiveStep()) {
        cursorString[i] = '|';
      }else if(i == seq.getActiveMenuStep()) {
          cursorString[i/*+ adder*/] ='^';
        } else {
          cursorString[i/* + adder*/] = ' ';
        }
      }
    } else {
      for(int i = 0; i < STEP_LENGTH ;i++) {
          cursorString[i] = ' ';
        }
    }
    display.setCursor(0,15);
    display.print(cursorString);
    display.setTextSize(1);
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
  outNote += String(seq.getNote(seq.getActiveMenuStep()));
  String outVelo= " VELO ";
  outVelo += String(seq.getVelocity(seq.getActiveMenuStep()));
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

void updateActiveStep(){

}
void updateSequence(){
  display.setTextSize(2);
  for(int i = 0; i  <STEP_LENGTH; i++) {
      if(seq.getGate(i) == true) {
        seqString[i/* + adder */] = 'X';
      } else {
        seqString[i/* + adder*/] = '-';
      }
  }
  display.setCursor(0,0);
  display.print(seqString);
  display.setTextSize(1);
}

void updateDisplay(){
  if(debugON) {
    Serial.print("update Display");
  }
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
        }
        if(mode == 2) { //velo
          seq.setVelocityUp();
        }
        if (mode == 0){
          seq.nextStep();
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
          }
          if(mode == 2) { //velo
            seq.setVelocityDown();
          }
          if (mode == 0){
            //turn right
            seq.prevStep();
          }
        }
    }
    oldPosition = newPosition;
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
  display.setTextColor(WHITE);
//display.setTextSize(2);

  // update display with all of the above graphics
  display.display();
  pinMode(buttonPin,INPUT);
  if(debugON) {
    //set baud rate for serial monitor
    Serial.begin(9600);
  } else {
    // set MIDI baud
    Serial.begin(31250);
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
       updateDisplay();
       break;
     case MIDI_CONT:
       seq.cont();
       break;
     case MIDI_CLOCK:
        updateDisplay();
        seq.clock();
       break;
     default:
       break;
   }
 }
}