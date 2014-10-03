//
//  KKPM.cpp
//  pm_mac
//
//  Created by Ken Kopecky II on 9/25/14.
//
//
//this is basically all taken from PortMIDI's passthru example

#include "KKPM.h"
#include "KensMIDIController.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "portmidi.h"
#include "pmutil.h"
#include "porttime.h"

namespace KKPM
{



bool useThreadProtection = false;
#define MIDI_SYSEX 0xf0
#define MIDI_EOX 0xf7

/* active is set true when midi processing should start */
int active = FALSE;
/* process_midi_exit_flag is set when the timer thread shuts down */
int process_midi_exit_flag;

PmStream *midi_in;
PmStream *midi_out;

/* shared queues */
#define IN_QUEUE_SIZE 1024
#define OUT_QUEUE_SIZE 1024
PmQueue *in_queue;
PmQueue *out_queue;
PmTimestamp current_timestamp = 0;
int thru_sysex_in_progress = FALSE;
int app_sysex_in_progress = FALSE;
PmTimestamp last_timestamp = 0;
std::string* gDeviceName;

const char* deviceName()	{return gDeviceName->c_str();}
bool isActive()				{return active;}

/* time proc parameter for Pm_MidiOpen */
PmTimestamp midithru_time_proc(void *info)
{
    return current_timestamp;
}


void doMIDI(int bytes)
{
	//quick and dirty int-to-bytes conversion!  ~ken
	union
	{
		int i;
		unsigned char b[4];
	}bb;
	
	bb.i = bytes;
//	printf("Bytes:  %i, %i\n", bb.b[2], bb.b[1]);
	MIDIControl::updateFromMidiSignal(bb.b[1], bb.b[2]);
	
}



/* timer interrupt for processing midi data.
   Incoming data is delivered to main program via in_queue.
   Outgoing data from main program is delivered via out_queue.
   Incoming data from midi_in is copied with low latency to  midi_out.
   Sysex messages from either source block messages from the other.
 */
void process_midi(PtTimestamp timestamp, void *userData)
{
    PmError result;
    PmEvent buffer; /* just one message at a time */

    current_timestamp++; /* update every millisecond */
    /* if (current_timestamp % 1000 == 0) 
        printf("time %d\n", current_timestamp); */

    /* do nothing until initialization completes */
    if (!active) {
        /* this flag signals that no more midi processing will be done */
        process_midi_exit_flag = TRUE;
        return;
    }

    /* see if there is any midi input to process */
    if (!app_sysex_in_progress) {
        do {
            result = Pm_Poll(midi_in);
            if (result) {
                int status;
                PmError rslt = (PmError)Pm_Read(midi_in, &buffer, 1);
                if (rslt == pmBufferOverflow) 
                    continue;
                assert(rslt == 1);

                /* record timestamp of most recent data */
                last_timestamp = current_timestamp;

                /* the data might be the end of a sysex message that
                   has timed out, in which case we must ignore it.
                   It's a continuation of a sysex message if status
                   is actually a data byte (high-order bit is zero). */
                status = Pm_MessageStatus(buffer.message);
                if (((status & 0x80) == 0) && !thru_sysex_in_progress) {
                    continue; /* ignore this data */
                }

                /* implement midi thru */
                /* note that you could output to multiple ports or do other
                   processing here if you wanted
                 */
//                printf("thru: %x\n", buffer.message);
				
				//either handle the message now, or queue it up for handing in the main thread
				if(!useThreadProtection)
				   doMIDI(buffer.message);
				else
				{
					Pm_Enqueue(in_queue, &buffer);
				}
                /* sysex processing */
                if (status == MIDI_SYSEX) thru_sysex_in_progress = TRUE;
                else if ((status & 0xF8) != 0xF8) {
                    /* not MIDI_SYSEX and not real-time, so */
                    thru_sysex_in_progress = FALSE;
                }
                if (thru_sysex_in_progress && /* look for EOX */
                    (((buffer.message & 0xFF) == MIDI_EOX) ||
                     (((buffer.message >> 8) & 0xFF) == MIDI_EOX) ||
                     (((buffer.message >> 16) & 0xFF) == MIDI_EOX) ||
                     (((buffer.message >> 24) & 0xFF) == MIDI_EOX))) {
                    thru_sysex_in_progress = FALSE;
                }
            }
        } while (result);
    }


    /* see if there is application midi data to process */
    while (!Pm_QueueEmpty(out_queue)) {
        /* see if it is time to output the next message */
        PmEvent *next = (PmEvent *) Pm_QueuePeek(out_queue);
        assert(next); /* must be non-null because queue is not empty */
        if (next->timestamp <= current_timestamp) {
            /* time to send a message, first make sure it's not blocked */
            int status = Pm_MessageStatus(next->message);
            if ((status & 0xF8) == 0xF8) {
                ; /* real-time messages are not blocked */
            } else if (thru_sysex_in_progress) {
                /* maybe sysex has timed out (output becomes unblocked) */
                if (last_timestamp + 5000 < current_timestamp) {
                    thru_sysex_in_progress = FALSE;
                } else break; /* output is blocked, so exit loop */
            }
            Pm_Dequeue(out_queue, &buffer);
            Pm_Write(midi_out, &buffer, 1);

            /* inspect message to update app_sysex_in_progress */
            if (status == MIDI_SYSEX) app_sysex_in_progress = TRUE;
            else if ((status & 0xF8) != 0xF8) {
                /* not MIDI_SYSEX and not real-time, so */
                app_sysex_in_progress = FALSE;
            }
            if (app_sysex_in_progress && /* look for EOX */
                (((buffer.message & 0xFF) == MIDI_EOX) ||
                 (((buffer.message >> 8) & 0xFF) == MIDI_EOX) ||
                 (((buffer.message >> 16) & 0xFF) == MIDI_EOX) ||
                 (((buffer.message >> 24) & 0xFF) == MIDI_EOX))) {
                app_sysex_in_progress = FALSE;
            }
        } else break; /* wait until indicated timestamp */
    }
}


void exit_with_message(const char *msg)
{
#define STRING_MAX 80
    char line[STRING_MAX];
    printf("%s\nType ENTER...", msg);
    fgets(line, STRING_MAX, stdin);
    exit(1);
}


void update()
{
	//only used with thread protection
	if(useThreadProtection == false) return;
    PmEvent buffer;

	 while (Pm_Dequeue(in_queue, &buffer) == 1)
		{
            /* printf("timestamp %d\n", buffer.timestamp); */
//            printf("main message %x\n", buffer.message);
			doMIDI(buffer.message);
		}


}

void finalize();


void init(bool threadProtection)
/* set up midi processing thread and open midi streams */
{
	useThreadProtection = threadProtection;
    /* note that it is safe to call PortMidi from the main thread for
       initialization and opening devices. You should not make any
       calls to PortMidi from this thread once the midi thread begins.
       to make PortMidi calls.
     */

    /* note that this routine provides minimal error checking. If
       you use the PortMidi library compiled with PM_CHECK_ERRORS,
       then error messages will be printed and the program will exit
       if an error is encountered. Otherwise, you should add some
       error checking to this code.
     */

    const PmDeviceInfo *info;
    int id;

    /* make the message queues */
    in_queue = Pm_QueueCreate(IN_QUEUE_SIZE, sizeof(PmEvent));
    assert(in_queue != NULL);
    out_queue = Pm_QueueCreate(OUT_QUEUE_SIZE, sizeof(PmEvent));
    assert(out_queue != NULL);

    /* always start the timer before you start midi */

	Pt_Start(1, &process_midi, 0); /* start a timer with millisecond accuracy */
    /* the timer will call our function, process_midi() every millisecond */
    
    Pm_Initialize();

    id = Pm_GetDefaultOutputDeviceID();
    info = Pm_GetDeviceInfo(id);
    if (info == NULL) {
        printf("Could not open default output device (%d).", id);
//        exit_with_message((const char*)"");
		return;
    }
    printf("Opening output device %s %s\n", info->interf, info->name);

    /* use zero latency because we want output to be immediate */
    Pm_OpenOutput(&midi_out, 
                  id, 
                  NULL /* driver info */,
                  OUT_QUEUE_SIZE,
                  &midithru_time_proc,
                  NULL /* time info */,
                  0 /* Latency */);

    id = Pm_GetDefaultInputDeviceID();
    info = Pm_GetDeviceInfo(id);
    if (info == NULL)
	{
//        printf("Could not open default input device (%d).", id);
//        exit_with_message("");
			return;
    }
    printf("Opening input device %s %s\n", info->interf, info->name);
	gDeviceName = new std::string(info->name);
    Pm_OpenInput(&midi_in, 
                 id, 
                 NULL /* driver info */,
                 0 /* use default input size */,
                 &midithru_time_proc,
                 NULL /* time info */);
    /* Note: if you set a filter here, then this will filter what goes
       to the MIDI THRU port. You may not want to do this.
     */
    Pm_SetFilter(midi_in, PM_FILT_ACTIVE | PM_FILT_CLOCK);

    active = TRUE; /* enable processing in the midi thread -- yes, this
                      is a shared variable without synchronization, but
                      this simple assignment is safe */
				
	atexit(finalize);

}


void finalize()
{
    /* the timer thread could be in the middle of accessing PortMidi stuff */
    /* to detect that it is done, we first clear process_midi_exit_flag and
       then wait for the timer thread to set it
     */
    process_midi_exit_flag = FALSE;
    active = FALSE;
    /* busy wait for flag from timer thread that it is done */
    while (!process_midi_exit_flag) ;
    /* at this point, midi thread is inactive and we need to shut down
     * the midi input and output
     */
    Pt_Stop(); /* stop the timer */
    Pm_QueueDestroy(in_queue);
    Pm_QueueDestroy(out_queue);

    Pm_Close(midi_in);
    Pm_Close(midi_out);

    Pm_Terminate();    
}

}