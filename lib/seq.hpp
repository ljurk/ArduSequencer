#include <Arduino.h>
#include "..\lib\midi.hpp"

#ifdef STEP_LENGTH
#else
  #define STEP_LENGTH 16
#endif

#define DEFAULT_VELOCITY 64

class sequencer {
private:
  int count = 0; // counter for midi ticks, 24 ticks are one quarter note
  byte speedDivider = 1; //1=24ticks,2=12ticks,4=6ticks
  byte defaultNote =  0;
  bool stopped = true;
  bool gate[STEP_LENGTH];
  byte notes[STEP_LENGTH];
  bool slide[STEP_LENGTH];
  byte velocity[STEP_LENGTH];
  byte activeStep= 0;
  byte oldStep= 0;
  byte oldMenuStep= 0;
  byte activeMenuStep=0;
  bool slideActive = false;
  bool seqDebug = false;
public:
  sequencer(bool dbg = false);
  int getDefaultNote();
  int getActiveStep();
  bool getStopped();
  bool getGate(int pos);
  byte getNote(int pos);
  bool getSlide(int pos);
  byte getVelocity(int pos);
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
  void setVelocityUp();
  void setVelocityDown();
  void setSlide();
  void setGate();
  void setNote();

  void start();
  void stop();
  void cont();
  void clock();
};
