#include "../lib/midi.hpp"

void sendMidi(byte channel, byte command, byte arg1, byte arg2) {
  if(command < 128) {
    // shift over command
    command <<= 4;
    // add channel to the command
    command |= channel;
  }
  // send MIDI data
/*debug
    Serial.println("MIDI");
    Serial.println(command);
    Serial.println(arg1);
    Serial.println(arg2);
    */
    Serial.write(command);
Serial.write(arg1);
Serial.write(arg2);

}

void sendNoteOn(byte note, byte velocity) {
  sendMidi(MIDI_CHANNEL, NOTE_ON, note, velocity);
}

void sendNoteOff(byte note) {
  sendMidi(MIDI_CHANNEL, NOTE_ON, note, 0);
}
