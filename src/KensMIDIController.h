#ifndef KENS_MIDI_CONTROLLER
#define KENS_MIDI_CONTROLLER
#include <string>


//this class listens for MIDI input and assigns it to sliders and dials
//the names of the controls map to the KORG nanoKONTROL2 Midi mixer
//but you could change them up to work with whatever device you have

class MIDIControl
{
public:

	//four possible states for a button.
	//this lets us handle press events and steady states
	enum ButtonState {OFF, ON, TOGGLE_OFF, TOGGLE_ON};

	MIDIControl();
	~MIDIControl()	{printData(); if(autowrite && filename != "") writeSavedState(filename);}
	static std::string getDeviceName();
	
	//as far as I can tell, MIDI doesn't have a state.  Just events.
	//In other words, I can't poll the value of a slider.  Just have to remember it.
	//use these to save and retrieve the state of the sliders and dials
	//between launches.
	

	static bool readSavedState(std::string fileName);
	static void saveAtExit();		//sets MIDIControl to write out its data at exit to the file it read it from
	static bool writeSavedState(std::string fileName);
	
	static bool active();
	static void printData();		//prints our data to the console so we can write it down or hardcode it

	/*  Call this once a frame to switch toggle states to steady states*/
	static void detoggle();

	/*get data for a slider, dial, or button.
			Sliders and dials are 0-1, buttons are ButtonStates
	*/
	static float slider(int which);
	static float dial(int which);
	
	//buttons
	static unsigned char s(int which);
	static unsigned char m(int which);
	static unsigned char r(int which);
	static unsigned char trackBack();
	static unsigned char trackForward();
	static unsigned char cycle();
	static unsigned char markerSet();
	static unsigned char markerForward();
	static unsigned char markerBack();
	static unsigned char rewind();
	static unsigned char fastForward();
	static unsigned char stop();
	static unsigned char play();
	static unsigned char record();
	
	static const int NUM_DIALS = 8;
	float mSliderValues[NUM_DIALS];
	float mDialValues[NUM_DIALS];
	
	//S, M, R buttons
	unsigned char mSValues[NUM_DIALS];
	unsigned char mMValues[NUM_DIALS];
	unsigned char mRValues[NUM_DIALS];
	
	union
	{
		unsigned char extraButtons[32];
		struct
		{
			
			//these use underscores instead of the m prefix
			//because of a macro for creating the getters in the .mm file
			unsigned char _trackBack;
			unsigned char _trackForward;
			unsigned char _cycle;
			unsigned char _markerSet;
			unsigned char _markerBack;
			unsigned char _markerForward;

			unsigned char _rewind;
			unsigned char _fastForward;
			unsigned char _stop;
			unsigned char _play;
			unsigned char _record;
		};
	};

	//called from the MiniMonitor to pass Midi events to the MIDIControl
	static void updateFromMidiSignal(unsigned char control, unsigned char value);
	static void init();				//create MIDI interface

	//called internally
	void _updateFromMidiSignal(unsigned char control, unsigned char value);

	//for auto-write
	private:
	bool autowrite;		//should we write our variables to disk when we're done?
	std::string filename;		//the file we loaded at the start

};
#endif

