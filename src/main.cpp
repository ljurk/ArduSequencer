//#include "..\lib\seq.hpp"
//#include "..\lib\midi.hpp"





//sequencer seq = sequencer(mainDebug);
/*~~~~~~~~~  LED   ~~~~~~~~~*/
#include "..\lib\led.hpp"
led mainSeq = led(mainDebug);

/*~~~~~~~  DISPLAY   ~~~~~~~*/
//#include "..\lib\display.hpp"
//displaySequencer mainSeq = displaySequencer(mainDebug);

/*~~~~~~  DEBUGMODE   ~~~~~~*/
bool mainDebug = false; //only accepts midi when set to true

void setup() {
  if(mainDebug) {
    //set baud rate for serial monitor
    Serial.begin(9600);
  } else {
    // set MIDI baud
    Serial.begin(31250);
  }
  mainSeq.startingAnimation();
}

void loop() {
  mainSeq.run();
}
