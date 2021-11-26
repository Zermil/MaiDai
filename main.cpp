#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>

// Windows specific
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <mmsystem.h>
#include <conio.h>

// For compliers that implement it
#pragma comment(lib, "winmm.lib")

static const std::string note_letters[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

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
    uint8_t octave;
    MIDINoteName note;
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
}

MIDINote get_note_from_number(const uint8_t& note_number)
{
    MIDINote midi_note;

    midi_note.note = static_cast<MIDINoteName>(note_number % 12);
    midi_note.octave = (note_number / 12) - 1;
    
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
	
	if (msg_indication == NOTE_OFF || msg_indication == NOTE_ON) {
	    uint8_t note_number = (dwParam1 >> 8 * 1) & 0xff;
	    uint8_t velocity = (dwParam1 >> 8 * 2) & 0xff;

	    MIDINote midi_note = get_note_from_number(note_number);
	    
	    printf("================\n");
	    printf("Note: %s Octave: %d, Velocity: %d\n", note_letters[midi_note.note].c_str(), midi_note.octave, velocity);
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
