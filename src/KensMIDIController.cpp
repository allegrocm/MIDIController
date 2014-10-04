#include "KensMIDIController.h"
#include "KKPM.h"
#include <math.h>

MIDIControl __gMIDIControl;
bool __gMIDIControlInited = false;

MIDIControl::MIDIControl()
{
	init();
	for(int i = 0; i < NUM_DIALS; i++)
	{
		mSliderValues[i] = 0;
		mDialValues[i] = 0;
		mSValues[i] = 0;
		mRValues[i] = 0;
		mMValues[i] = 0;
	}
	memset(extraButtons, 0, sizeof(extraButtons));
	autowrite = false;
	filename = "";
}

void MIDIControl::saveAtExit()
{
	__gMIDIControl.autowrite = true;

}

void MIDIControl::init()
{

	if(__gMIDIControlInited) return;

	__gMIDIControlInited = true;
	KKPM::init(false);


}

bool MIDIControl::active()
{
	return KKPM::isActive();
}

std::string MIDIControl::getDeviceName()
{
	return KKPM::deviceName();
}



float MIDIControl::slider(int which)
{
//	which--;
	if(which < 0 || which >= NUM_DIALS) return 0;
	return __gMIDIControl.mSliderValues[which];
}

float MIDIControl::dial(int which)
{
//	which--;

	if(which < 0 || which >= NUM_DIALS) return 0;
	return __gMIDIControl.mDialValues[which];
}

unsigned char MIDIControl::s(int which)
{
//	which--;

	if(which < 0 || which >= NUM_DIALS) return 0;
	return __gMIDIControl.mSValues[which];
}

unsigned char MIDIControl::m(int which)
{
//	which--;

	if(which < 0 || which >= NUM_DIALS) return 0;
	return __gMIDIControl.mMValues[which];
}

unsigned char MIDIControl::r(int which)
{
//	which--;

	if(which < 0 || which >= NUM_DIALS) return 0;
	return __gMIDIControl.mRValues[which];
}

//bunch of similar functions, let's macro them
#define buttonGet(b) unsigned char MIDIControl::b() {return __gMIDIControl._##b;}
buttonGet(trackBack)
buttonGet(trackForward)
buttonGet(cycle)
buttonGet(markerSet)
buttonGet(markerBack)
buttonGet(markerForward)

buttonGet(rewind)
buttonGet(fastForward)
buttonGet(stop)
buttonGet(play)
buttonGet(record)




void MIDIControl::printData()
{
	printf("____________________________________________________\n");
	printf("MIDI Controller Data:\n");

	for(int i = 0; i < NUM_DIALS; i++)
		printf("%+.2f %s", dial(i+1), i == 3 ? " | " : "");
	printf("\n");
	for(int i = 0; i < NUM_DIALS; i++)
		printf("%+.2f %s", slider(i+1), i == 3 ? " | " : "");
	printf("\n");
	printf("____________________________________________________\n");



}

void MIDIControl::updateFromMidiSignal(unsigned char control, unsigned char value)
{
	__gMIDIControl._updateFromMidiSignal(control, value);
}

void MIDIControl::_updateFromMidiSignal(unsigned char control, unsigned char value)
{
//	printf("Control %i:  %i\n", control, value);
	
	
	//is this a button state?  figure that out now
	ButtonState toggle = (value == 0) ? TOGGLE_OFF : TOGGLE_ON;
	
	if(control < 8)		//sliders are 0-7
	{
		__gMIDIControl.mSliderValues[control] = (float)value / 127;
//		printf("Slider %i set to %.2f\n", control, slider(control));
	}
	else if(control >= 16 && control < 24)
		__gMIDIControl.mDialValues[control-16] = (float)value / 127;
	else if(control >= 32 && control <= 39)
		__gMIDIControl.mSValues[control-32] = toggle;
	else if(control >= 48 && control <= 55)
		__gMIDIControl.mMValues[control-48] = toggle;
	else if(control >= 64 && control <= 71)
		__gMIDIControl.mRValues[control-64] = toggle;
		
	else if(control == 58) __gMIDIControl._trackBack = toggle;
	else if(control == 59) __gMIDIControl._trackForward = toggle;
	else if(control == 46) __gMIDIControl._cycle = toggle;
	else if(control == 61) __gMIDIControl._markerBack = toggle;
	else if(control == 62) __gMIDIControl._markerForward = toggle;
	else if(control == 60) __gMIDIControl._markerSet = toggle;
	else if(control == 43) __gMIDIControl._rewind = toggle;
	else if(control == 44) __gMIDIControl._fastForward = toggle;
	else if(control == 42) __gMIDIControl._stop = toggle;
	else if(control == 41) __gMIDIControl._play = toggle;
	else if(control == 45) __gMIDIControl._record = toggle;
	else printf("Don't know what this is\n");
	
	//buttons:
	//S buttons are 32-39
	//M buttons are 48-55
	//R buttons are 64-71
	//Track < and > are 58, 59
	//cycle:  46
	//Marker Set, <, >:  60, 61, 62
	//<<, >>, stop, play, record:  43, 44, 42, 41, 45
	
}

bool MIDIControl::writeSavedState(std::string filename)
{
	//just write all our values.  not buttons though.  we don't keep those
	FILE* f = fopen(filename.c_str(), "w");
	if(!f) return false;

	for(int i = 0; i < NUM_DIALS; i++)
		fprintf(f, "%.3f ", slider(i));

	for(int i = 0; i < NUM_DIALS; i++)
		fprintf(f, "%.3f ", dial(i));
	
	fclose(f);
	
	return true;
}


bool MIDIControl::readSavedState(std::string filename)
{
	__gMIDIControl.filename = filename;
	//just write all our values.  not buttons though.  we don't keep those
	FILE* f = fopen(filename.c_str(), "r");
	if(!f) return false;

	for(int i = 0; i < NUM_DIALS; i++)
		fscanf(f, "%f ", &__gMIDIControl.mSliderValues[i]);

	for(int i = 0; i < NUM_DIALS; i++)
		fscanf(f, "%f ", &__gMIDIControl.mDialValues[i]);
	
	
	fclose(f);
	
	return true;
}

#define detoggleState(s) (s == TOGGLE_OFF)? OFF : (s == TOGGLE_ON) ? ON : s

void MIDIControl::detoggle()
{
	for(int i = 0; i < NUM_DIALS; i++)
	{
		__gMIDIControl.mSValues[i] = detoggleState(__gMIDIControl.mSValues[i]);
		__gMIDIControl.mMValues[i] = detoggleState(__gMIDIControl.mMValues[i]);
		__gMIDIControl.mRValues[i] = detoggleState(__gMIDIControl.mRValues[i]);
	}
	
	for(int i = 0; i < 32; i++)
		__gMIDIControl.extraButtons[i] = detoggleState(__gMIDIControl.extraButtons[i]);
}

