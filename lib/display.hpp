#include <Encoder.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "../lib/seq.hpp"
#include "../lib/midi.hpp"


class displaySequencer{
private:
  //display symbols
  const char emptyStepSymbol = '.';
  const char filedStepSymbol = ',';
  const char cursorEmptyStepSymbol = ':';
  const char cursorFiledStepSymbol = ';';
  const char activeStepSymbol = '|';
  const char quarterSymbol = '=';

  int count = 0;
  bool debugDisplay = false;
  LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,20,4);
  const int encoderButtonPin = 13;
  const int modeButtonPin = 8;
  Encoder myEnc = Encoder(2, 3);

  struct channel {
    int pin;
    bool muted;
    bool buttonPressed;
    bool buttonState;
  };
  channel channels[NUMBER_OF_CHANNELS];
  unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
  int debounceDelay = 300;
  bool encoderButtonState = false;
  bool encoderButtonPressed = false;
  bool modeButtonState = false;
  bool modeButtonPressed = false;
  long oldEncoderPos  = -999;
  long newEncoderPos =  -999;
  String seqString ="0000000000000000";
  String cursorString ="0000000000000000";
  // 0=sequencer;1=velocity;3=euclidean
  byte mode = 0;
  bool somethingChanged = true;
  bool activeStepChanged = true;
  bool cursorChanged = true;
  bool sequenceChanged = true;
  bool valuesChanged = true;
  sequencer seq;

  void step(byte pos);
  void updateValues();
  void updateCursor();
  void updateSequence();
  void updateDisplay();
  void checkInputs();
  void updateActiveStep();
public:
  displaySequencer(bool debug);
  void startingAnimation();
  void run();

};
