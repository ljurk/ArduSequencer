#include <Arduino.h>
//for MIDI IN
#define MIDI_START 250
#define MIDI_STOP 252
#define MIDI_CONT 251
#define MIDI_CLOCK 248
#define NOTE_OFF 8
#define NOTE_ON 9
//for MIDI OUT
#ifdef MIDI_CHANNEL
#else
  #define MIDI_CHANNEL 11 //0-15 represents channels 1-16
#endif

void sendMidi(byte channel, byte command, byte arg1, byte arg2);
void sendNoteOn(byte note, byte velocity);
void sendNoteOff(byte note);
