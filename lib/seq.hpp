#include <Arduino.h>
#include "..\lib\midi.hpp"

#if STEP_LENGTH
  #include <Arduino.h>
#else
  #define STEP_LENGTH 8
#endif
#define NOTE_OFF 8
#define NOTE_ON 9
#define DEFAULT_VELOCITY 64

class sequencer {
private:

public:
  sequencer(bool dbg = false);
  int getDefaultNote();
  int getActiveStep();
  bool getStopped();
  bool getGate(int pos);
  byte getNote(int pos);
  bool getSlide(int pos);
  byte getOldStep();
  byte getOldMenuStep();
  bool getSlideActive();
  byte getActiveMenuStep();

  void defaultNoteUp();
  void defaultNoteDown();
  void resetSequence();
  void nextStep();
  void prevStep();
  void step();

  void noteDown();
  void noteUp();
  void setSlide();
  void setGate();
  void setNote();

  void start();
  void stop();
  void cont();
  void clock();
};
