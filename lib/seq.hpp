#include <Arduino.h>
#include "../lib/midi.hpp"

#ifdef STEP_LENGTH
#else
  #define STEP_LENGTH 16
#endif

#ifdef NUMBER_OF_CHANNELS
#else
  #define NUMBER_OF_CHANNELS 4
#endif

#define DEFAULT_VELOCITY 64
#define NEXT 1
#define PREV 0

struct seqChannel {
  bool gate[STEP_LENGTH];
  //byte notes[STEP_LENGTH];
  byte note;
  String noteText;
  bool slide[STEP_LENGTH];
  byte velocity[STEP_LENGTH];
  byte length;
  byte numberOfGates;
  byte activeStep;
  byte oldStep;
  bool mute;
};

class sequencer {
private:
  int count = 0; // counter for midi ticks, 24 ticks are one quarter note
  byte speedDivider = 2; //1=24ticks,2=12ticks,4=6ticks
  byte defaultNote =  0;
  bool stopped = true;

  seqChannel chan[NUMBER_OF_CHANNELS];
  byte cursorPos = 0;
  byte activeChannel = 0;
  bool slideActive = false;
  bool seqDebug = false;
public:
  sequencer(bool dbg = false);

  //get
  int getDefaultNote();
  byte getActiveStep(byte channel);
  byte getOldStep(byte channel);
  bool getStopped();
  bool getGate(byte channel,byte pos);
  byte getNote(byte channel/*,int pos*/);
  String getNoteText(byte channel/*,int pos*/);
  byte getNoteTextLength(byte channel/*,int pos*/);
  bool getSlide(byte channel,byte pos);
  byte getVelocity(byte channel,byte pos);
  byte getLength(byte channel);
  byte getOldStep();
  byte getOldCursorPos();
  bool getSlideActive();
  byte getCursorPos();
  byte getActiveChannel();
  bool getMute(byte channel);

  void defaultNoteUp();
  void defaultNoteDown();
  void resetSequence();
  void step(byte channel);

  //set, always depends on activeChannel and cursorPos
  void euclidAddGate();
  void euclidRemoveGate();
  void setNoteDown();
  void setNoteUp();
  void setVelocityUp(int steps = 1);
  void setVelocityDown(int steps = 1);
  void setLength(int steps);
  void setSlide();
  void setGate();
  void setNote();
  void setMute(byte channel);
  //navigation
  void setActiveChannel(byte channel);
  void setCursorPos(bool direction); //true = forwards, false = backwards
  void setCursorPosDirect(byte pos);

  void start();
  void stop();
  void cont();
  void clock();
};
