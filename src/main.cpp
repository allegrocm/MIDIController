#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#include <Glut/glut.h>
#else
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <math.h>
#include <string>
#include <iostream>

#include "KensMIDIController.h"


float aspect = 1.333;
int screenHeight = 768;
int screenWidth = 1024;
bool fullScreen = false;

//for animating
float gAngle[] = {0, 0, 0, 0, 0, 0, 0, 0};



//this is a freakin handy little function to draw GLUT text on the screen
void drawCenteredStringOnScreen(int x, int y, const char* format, ...)
{
	char stringData[1024];
	va_list args;
	va_start(args, format);
	vsprintf(stringData, format, args);
	glViewport(0, 0, screenWidth, screenHeight);
	glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			glOrtho(0, screenWidth, 0, screenHeight, -1, 1);

			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			float length = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)stringData);
			glRasterPos3i(x-length/2, screenHeight - y, 0);
			for( int i=0; i<1024 && stringData[i] != 0; i++ )
				glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, stringData[i] );
			glEnable(GL_LIGHTING);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	glPopMatrix();


}

//this is a freakin handy little function to draw GLUT text on the screen
void drawStringOnScreen(int x, int y, const char* format, ...)
{
	char stringData[1024];
	va_list args;
	va_start(args, format);
	vsprintf(stringData, format, args);
	glViewport(0, 0, screenWidth, screenHeight);
	glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			glOrtho(0, screenWidth, 0, screenHeight, -1, 1);

			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glRasterPos3i(x, screenHeight - y, 0);
			for( int i=0; i<1024 && stringData[i] != 0; i++ )
				glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, stringData[i] );
			glEnable(GL_LIGHTING);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	glPopMatrix();


}


void drawFrameRate()
{
		glColor3f(1,1,1);
		static float t=0;
		static float fps=2;
		static float deltat=.2;
		static int reps=0;
		float tnow=glutGet(GLUT_ELAPSED_TIME);
		reps++;

		if(tnow - t > 500)		//update every 500 ms
		{
			deltat=tnow-t;
			t=tnow;
			fps=1.f * reps/deltat*1000;
			reps = 0;
		}
		drawStringOnScreen(20, 20, "Frame Rate:  %.2f", fps);
		
}

void setLight(float x, float y, float z)
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float lightPosition[] = {x, y, z, 1};
	glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);
}


void setupGL()
{
	//set up our OpenGL state so we can draw some cubes
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, aspect, 0.2, 200.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 20, 0, 0, 0, 0, 1, 0);
	
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	setLight(100, 100, 50);

}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//calculate elapsed time
	static int lastTime = glutGet(GLUT_ELAPSED_TIME);
	int thisTime = glutGet(GLUT_ELAPSED_TIME);
	float dt = 0.001 * ((float) thisTime - lastTime);
	lastTime = thisTime;
	if(MIDIControl::active())
		drawCenteredStringOnScreen(screenWidth/2, screenHeight*0.3 - 20, "Detected MIDI device:  %s", MIDIControl::getDeviceName().c_str());
	else
			drawCenteredStringOnScreen(screenWidth/2, screenHeight*0.3 - 20, "No MIDI device found.  Plug it in and restart the app.");
	drawCenteredStringOnScreen(screenWidth/2, screenHeight*0.3,
		"Use the dials on your MIDI device to control speed and sliders to control height");
	
	for(int i = 0; i < 8; i++)
	{
		float separation = 4;
		glPushMatrix();
			
			float spinSpeed = MIDIControl::dial(i);
			float height = MIDIControl::slider(i);
			gAngle[i] += spinSpeed * dt * 360;
			glTranslatef(separation * (-3.5 + i), 2.0 * height, 0);
			glRotatef(gAngle[i], i%2, i%3, i%4);
			glColor3f(1, 0, 0);
			glutSolidCube(1);
		glPopMatrix();
	}
	
	//a quick macro for choosing a color based on button state
	//four colors to show the button states
	unsigned int colorForStates[] = {0xff0000ff, 0xff00ff00, 0xffff0000, 0xff00ffff};
#define setColor(c) 		glColor4ubv((unsigned char*)&colorForStates[c])
	//now show button stuff
	for(int i = 0; i < 8; i++)
	{
		unsigned char s = MIDIControl::s(i);
		unsigned char m = MIDIControl::m(i);
		unsigned char r = MIDIControl::r(i);
		setColor(s);
		drawStringOnScreen(20 + i * 60, 100, "S%i", i);
		setColor(m);
		drawStringOnScreen(20 + i * 60, 130, "M%i", i);
		setColor(r);
		drawStringOnScreen(20 + i * 60, 160, "R%i", i);
	}
	
	//now draw the other controls, arranged kinda how they are on the nanoKontrol2
	int px = 500;
	int py = screenHeight * 0.7;
	float scale = 600.0;
	
	setColor(MIDIControl::trackBack());
	drawStringOnScreen(px + 0 * scale, py + 0 * scale, "( < )");
	setColor(MIDIControl::trackForward());
	drawStringOnScreen(px + .1 * scale, py + 0 * scale, "( > )");

	setColor(MIDIControl::cycle());
	drawStringOnScreen(px + 0 * scale, py + 0.1 * scale, "(cycle)");

	setColor(MIDIControl::markerSet());
	drawStringOnScreen(px + .2 * scale, py + 0.1 * scale, "(set)");
	setColor(MIDIControl::markerBack());
	drawStringOnScreen(px + .3 * scale, py + 0.1 * scale, "( < )");
	setColor(MIDIControl::markerForward());
	drawStringOnScreen(px + .4 * scale, py + .1 * scale, "( > )");

	setColor(MIDIControl::rewind());
	drawStringOnScreen(px + .0 * scale, py + .2 * scale, "[<<]");
	setColor(MIDIControl::fastForward());
	drawStringOnScreen(px + .1 * scale, py + .2 * scale, "[>>]");
	setColor(MIDIControl::stop());
	drawStringOnScreen(px + .2 * scale, py + .2 * scale, "[  ]");
	setColor(MIDIControl::play());
	drawStringOnScreen(px + .3 * scale, py + .2 * scale, "[ > ]");
	setColor(MIDIControl::record());
	drawStringOnScreen(px + .4 * scale, py + .2 * scale, "[ o ]");


	MIDIControl::detoggle();		//call this to
	drawFrameRate();
#undef setColor		//don't need this anymore
	glutSwapBuffers();
}





void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 'f':
			if(!fullScreen)	glutFullScreen();
			else glutReshapeWindow(1024, 768);
			fullScreen = !fullScreen;
			break;
			
		case 27: exit(0); break;	//ESC 
	
	}

	glutPostRedisplay();
}



void timer(int bah)
{
	//maintain about 30 fps
	glutTimerFunc(1000/30, timer, 0);
	glutPostRedisplay();

}

void resizify(int x, int y)
{
	aspect = (float)x / y;
	screenHeight = y;
	screenWidth = x;
	glViewport(0, 0, x, y);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutPostRedisplay();
}







std::string saveFileName = 
#ifdef _WIN32
	"./KKMidiValues";
#else
	"/tmp/KKMidiValues";
#endif

void writeMIDIValues()
{
	bool b = MIDIControl::writeSavedState(saveFileName);
	printf("Write:  %i\n", b);
}

int main(int argc, char *argv[])
{
	atexit(writeMIDIValues);

  
	printf("\n\nStarting GLUT loop....\n");
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(1024, 768);
	glutCreateWindow( "Ken Carn Kick!" );
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(resizify);
	glutTimerFunc(0, timer, 0);
	glutDisplayFunc(display);
	setupGL();
	


	bool b = MIDIControl::readSavedState(saveFileName);
	printf("read:  %i\n", b);

	glutMainLoop();
	return 0;

  //set up a virtual device
}

