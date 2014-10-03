//
//  KKPM.h
//  pm_mac
//
//  Created by Ken Kopecky II on 9/25/14.
//
//

#ifndef __pm_mac__KKPM__
#define __pm_mac__KKPM__



namespace KKPM
{
	//init port MIDI and Ken's MIDI stuff.  or FROM Ken's MIDI stuff
	void init(bool threadProtection);
	void update();		//processes MIDI signals if we're not handing them in PortMIDI's thread
	const char* deviceName();
	bool isActive();
}
#endif /* defined(__pm_mac__KKPM__) */
