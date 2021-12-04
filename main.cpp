#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>

// Windows specific
#define WIN32_LEAN_AND_MEAN
#define WINDIO_IMPLEMENTATION

#include "./windio/windio.hpp"
#include <Windows.h>
#include <mmsystem.h>
#include <conio.h>

// For compliers that implement it
#pragma comment(lib, "winmm.lib")

// TODO(Aiden): When no MIDI device is detected use keyboard.

static const std::string note_letters[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
static constexpr double FREQ_RATIO = 1.05946309436;
static output_settings settings;
static std::vector<double> frequencies;

enum MIDIEvent : uint8_t {
    NOTE_OFF = 0x80,
    NOTE_ON = 0x90,
};

enum MIDINoteName {
    NOTE_C = 0,
    NOTE_CS,
    NOTE_D,
    NOTE_DS,
    NOTE_E,
    NOTE_F,
    NOTE_FS,
    NOTE_G,
    NOTE_GS,
    NOTE_A,
    NOTE_AS,
    NOTE_B,
};

struct MIDINote {
    MIDINoteName note;
    int octave;
    double frequency;
};

void no_error(const MMRESULT& result)
{
    // TODO(Aiden): Properly handle ERROR(s).
    
    if (result != MMSYSERR_NOERROR) {
      fprintf(stderr, "Error calling one of the MIDI functions!");
      exit(1);
    }
} 

void cleanup(const HMIDIIN& handle)
{
    if (handle) {
      midiInStop(handle);
      midiInClose(handle);
    }

    windioDestroy();
}

MIDINote get_note_from_number(uint8_t note_number)
{
    MIDINoteName note = static_cast<MIDINoteName>(note_number % 12);
    int octave = (note_number / 12) - 1;

    int half_steps_count = note + 12 * octave - 57;
    double frequency = 440 * pow(FREQ_RATIO, half_steps_count);
    
    MIDINote midi_note = {
      note,
      octave,
      frequency
    };
    
    return midi_note;
}

// TODO(Aiden): Clear the sound, no more pops, make sure chords sound good.
void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    // Not interesed currently
    (void) hMidiIn;
    (void) dwInstance;
    (void) dwParam2; // Timestamp

    if (wMsg == MIM_DATA) {
      uint8_t msg_indication = (dwParam1 >> 8 * 0) & 0xff;
      uint8_t note_number = (dwParam1 >> 8 * 1) & 0xff;
      uint8_t velocity = (dwParam1 >> 8 * 2) & 0xff;
	      
      switch (msg_indication) {
	  case NOTE_ON: {	      
	      MIDINote midi_note = get_note_from_number(note_number);

	      frequencies.push_back(midi_note.frequency);
	      windioPlayMultiple(frequencies);
	      
	      printf("================\n");
	      printf("Note: %s Octave: %d Frequency: %.2f, Velocity: %d\n",
		     note_letters[midi_note.note].c_str(),
		     midi_note.octave,
		     midi_note.frequency,
		     velocity);
	  } break;

	  case NOTE_OFF: {
	      MIDINote midi_note = get_note_from_number(note_number);
	      frequencies.erase(std::remove(frequencies.begin(), frequencies.end(), midi_note.frequency), frequencies.end());
	      
	      windioPlayMultiple(frequencies);
	  } break;
      }
    }
}

int main(int argc, char* argv[])
{
    // TODO(Aiden): Used later
    (void) argc;
    (void) argv;
    
    UINT devices = midiInGetNumDevs();
    MIDIINCAPS caps = {};
    HMIDIIN handle_in;

    windioInitialize(&settings);
    no_error(midiInGetDevCaps(0, &caps, sizeof(caps)));

    printf("Input MIDI devices found: %d\n", devices);
    printf("Device currently used: %s\n", caps.szPname);
    
    no_error(midiInOpen(&handle_in, 0, reinterpret_cast<DWORD_PTR>(MidiInProc), 0, CALLBACK_FUNCTION));
    {
      bool is_playing = true;
      char c;

      no_error(midiInStart(handle_in));
      printf("Started receiving input, press [Q] to exit...\n");

      while (is_playing) {
          c = _getch();

          switch (c) {
            case 113:
            case 81:
                is_playing = false;
                break;
          }
      }
    }

    cleanup(handle_in);
    return 0;
}
