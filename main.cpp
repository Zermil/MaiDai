#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>

// Windows specific
#define WINDIO_IMPLEMENTATION
#include "./windio/windio.hpp"

#include <Windows.h>
#include <mmsystem.h>
#include <conio.h>

// For compliers that implement it
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "user32.lib")

static constexpr double FREQ_RATIO = 1.05946309436;

static const char* note_letters[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
static windio_settings settings;
static std::vector<float> frequencies;

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

struct MIDINote
{
    MIDINoteName note;
    int octave;
    float frequency;
};

MIDINote get_note_from_number(uint8_t note_number)
{
    MIDINote midi_note = {};
    
    midi_note.note = static_cast<MIDINoteName>(note_number % 12);
    midi_note.octave = (note_number / 12) - 1;

    // NOTE(Aiden): we subtract 57 because it is A4 --> 440Hz
    int half_steps_count = (12 * midi_note.octave) + midi_note.note - 57;
    midi_note.frequency = 440 * pow(FREQ_RATIO, half_steps_count);
    
    return midi_note;
}

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
	      
              printf("================\n");
              printf("Note: %s Octave: %d Frequency: %.2f, Velocity: %d\n",
                     note_letters[midi_note.note],
                     midi_note.octave,
                     midi_note.frequency,
                     velocity);
          } break;

          case NOTE_OFF: {
              MIDINote midi_note = get_note_from_number(note_number);
              frequencies.erase(std::remove(frequencies.begin(), frequencies.end(), midi_note.frequency), frequencies.end());	      
          } break;
      }

      windioPlayMultiple(settings, frequencies, WAVE_SIN);
    }
}

int main(int argc, char* argv[])
{
    // NOTE(Aiden): Used later
    (void) argc;
    (void) argv;

    UINT devices = midiInGetNumDevs();
    MIDIINCAPS caps = {};
    HMIDIIN handle;
    MMRESULT device_result = midiInGetDevCaps(0, &caps, sizeof(caps));

    // TODO(#1): When no MIDI device is detected use keyboard.
    assert((device_result == MMSYSERR_NOERROR) && "[ERROR]: No MIDI device found\n");
    windioInitializeSettings(settings);
    
    printf("Input MIDI devices found: %d\n", devices);
    printf("Device currently used: %s\n", caps.szPname);

    MMRESULT open_result = midiInOpen(&handle, 0, reinterpret_cast<DWORD_PTR>(MidiInProc), 0, CALLBACK_FUNCTION);
    assert((open_result == MMSYSERR_NOERROR) && "[ERROR]: MIDI device could not be opened\n");

    {
        bool is_playing = true;
        char c;

        MMRESULT start_result = midiInStart(handle);
        assert((start_result == MMSYSERR_NOERROR) && "[ERROR]: Could not properly start MIDI procedure\n");
        
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

    midiInStop(handle);
    midiInClose(handle);
    
    return 0;
}
