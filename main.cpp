#include <cstdio>
#include <cstdlib>
#include <cstdint>

// Windows specific
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <mmsystem.h>
#include <conio.h>

// For compliers that implement it
#pragma comment(lib, "winmm.lib")

void no_error(const MMRESULT& result)
{
    // TODO(Aiden): Properly handle ERROR(s).
    
    if (result != MMSYSERR_NOERROR) {
	fprintf(stderr, "Error with MIDI device!");
	exit(1);
    }
}

void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    // Not interesed currently
    (void) hMidiIn;
    (void) dwInstance;

    if (wMsg == MIM_DATA) {
	printf("================\n");

	uint8_t msg_indication = (dwParam1 >> 8 * 0) & 0xff;
	uint8_t msg_param1 = (dwParam1 >> 8 * 1) & 0xff;
	uint8_t msg_param2 = (dwParam1 >> 8 * 2) & 0xff;
	
	printf("MIDI bytes: %d, %d, %d\n",
	       msg_indication,
	       msg_param1,
	       msg_param2);
	
	printf("Timestamp: %d\n", static_cast<uint32_t>(dwParam2));
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

	no_error(midiInStop(handle_in));
    }
    no_error(midiInClose(handle_in));
    
    return 0;
}
