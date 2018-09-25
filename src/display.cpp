#include <Encoder.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "../lib/display.hpp"
// OLED display TWI address
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(-1);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
#define STEP_LENGTH 16
const int buttonPin = 4;
Encoder myEnc(2, 3);
int buttonState = 0;
bool buttonPressed = false;
long oldPosition  = -999;
long newPosition =  -999;
byte actualStep = 0;
bool gate[STEP_LENGTH + 1];
String seqString ="00000000 00000000";
String cursorString ="00000000 00000000";
byte mode = 0;
int velocity = 127;
int note = 60;

void showValues(){
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
  outVelo += String(velocity);
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
  //showValues(false);
  Serial.println(seqString);
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(seqString);
  display.display();
}

void showCursor(byte pos) {
  Serial.println(actualStep);
  display.clearDisplay();
  updateSequence();
  showValues();
  byte adder = 0;

  if(actualStep >= (STEP_LENGTH / 2)) {
    adder = 1;
  } else {
    adder = 0;
  }
  Serial.println(actualStep);
  Serial.println(adder);
  if(mode == 0 ) {
    for(int i = 0; i < STEP_LENGTH +1;i++) {
      if(i == STEP_LENGTH) {
        cursorString[i] = ' ';
      }
      if(i == actualStep) {
          cursorString[i+ adder] ='^';
        } else {
          cursorString[i + adder] = ' ';
        }

    }
    display.setCursor(0,15);
    display.print(cursorString);
    /*switch(pos) {
      case 0:
        display.setCursor(0,15);
        display.print("^       ");
        break;
      case 1:
        display.setCursor(0,15);
        display.print(" ^      ");
        break;
      case 2:
        display.setCursor(0,15);
        display.print("  ^     ");
        break;
      case 3:
        display.setCursor(0,15);
        display.print("   ^    ");
        break;
      case 4:
        display.setCursor(0,15);
        display.print("     ^   ");
        break;
      case 5:
        display.setCursor(0,15);
        display.print("      ^  ");
        break;
      case 6:
        display.setCursor(0,15);
        display.print("       ^ ");
        break;
      case 7:
        display.setCursor(0,15);
        display.print("        ^");
        break;

      default:
        break;
    }*/

  }
}

void setStep() {
  gate[actualStep] = !gate[actualStep];
  updateSequence();
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
          if(note == 127) {
            note = 60;
          } else {
              note ++;
          }
        }
        if(mode == 2) { //velo
          if(velocity == 127) {
            velocity = 0;
          } else {
              velocity ++;
          }
        }
        if (mode == 0){
          //turn right
          actualStep ++;
          if (actualStep == STEP_LENGTH + 1) {
            actualStep = 0;
          }
        }
      }
      showCursor(actualStep);
    } else if(newPosition < oldPosition - 1){
        //turn right
        if (buttonState == HIGH){
          mode--;
          if(mode == 255) {
            mode = 2;
          }
        } else{
          if(mode == 1) { //velo
            if(note == 60) {
              note = 127;
            } else {
                note --;
            }
          }
          if(mode == 2) { //velo
            if(velocity == 0) {
              velocity = 127;
            } else {
                velocity --;
            }
          }
          if (mode == 0){
            //turn right
            actualStep--;
            if (actualStep == 255) {
              actualStep = STEP_LENGTH;
            }
          }
        }
        showCursor(actualStep);
    }
    oldPosition = newPosition;

    /*display.clearDisplay();
    Serial.println(newPosition);
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(27,30);

    String myString = String(newPosition);
    display.print(myString);*/
  }
  if (buttonState == HIGH && buttonPressed == false) {
    setStep();
    showCursor(actualStep);
    buttonPressed = true;
  } else if(buttonState == LOW) {
    buttonPressed = false;
  }

  display.display();
}
