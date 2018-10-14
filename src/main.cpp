//#include "..\lib\seq.hpp"
//#include "..\lib\midi.hpp"
//#include "LiquidCrystal_I2C.h"
#include <Arduino.h>

/*~~~~~~  DEBUGMODE   ~~~~~~*/
bool mainDebug = true; //only accepts midi when set to true

//LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,20,4);

//sequencer seq = sequencer(mainDebug);
/*~~~~~~~~~  LED   ~~~~~~~~~*/
//#include "..\lib\led.hpp"
//led mainSeq = led(mainDebug);

/*~~~~~~~  DISPLAY   ~~~~~~~*/
#include "..\lib\display.hpp"
//displaySequencer mainSeq;

displaySequencer* mainSeq;

void setup() {
  mainSeq = new displaySequencer(mainDebug);

  if(mainDebug) {
    //set baud rate for serial monitor
    Serial.begin(9600);
  } else {
    // set MIDI baud
    Serial.begin(31250);
  }

  mainSeq->startingAnimation();
}

void loop() {
  mainSeq->run();
}
