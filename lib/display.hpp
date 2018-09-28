#include "..\lib\Encoder.h"
//#include <Wire.h>
#include "..\lib\Adafruit_SSD1306.h"
#include <Adafruit_GFX.h>
#include "..\lib\seq.hpp"
#include "..\lib\midi.hpp"

#define OLED_ADDR   0x3C

class displaySequencer{
private:
  bool debugDisplay = false;
  Adafruit_SSD1306 display = Adafruit_SSD1306(-1);
  const int buttonPin = 10;
  Encoder myEnc = Encoder(2,3);
  unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
  int debounceDelay = 300;
  int buttonState = 0;
  bool buttonPressed = false;
  long oldPosition  = -999;
  long newPosition =  -999;
  String seqString ="00000000";
  String cursorString ="00000000";
  byte mode = 0;
  sequencer seq;
public:
  displaySequencer(bool debug);
  void step(byte pos);
  void startingAnimation();
  void updateValues();
  void updateCursor();
  void updateSequence();
  void updateDisplay();
  void run();
  void checkInputs();
  void updateActiveStep();
};
