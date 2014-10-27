//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
//
//	OpenGL/ GLUT 'starter' code to demonstrate 3D transforms;
//					play with this to learn how GL_PROJECTION and GL_MODELVIEW
//					matrices affect how 3D vertices appear on-screen.
//	OVERVIEW:
//		OpenGL transforms all vertices first by GL_MODELVIEW matrix and then by
//		the GL_PROJECTION matrix before display on screen. These matrices act as
//		converters fr 'model' space to 'world' space to 'eye' space:
//
//		model space --GL_MODELVIEW ---> world space --GL_PROJECTION--> eye space
//		(vertices)  --GL_MODELVIEW --->  (ground)   --GL_PROJECTION--> (film)
//		
//		Note that we're used to thinking of *everything* in world space, both
//		the we view and the camera that views them. The behavior of MODELVIEW
//		matrix makes sense to most people--it transforms basic shapes to their 
//		world-space positions.  But the GL_PROJECTION is usually confusing; this
//		matrix transforms away from world-space and into eye-space. The origin
//		of eye-space is the cameras position, the camera's direction of view
//		is the -Z direction, and camera image x,y define eye-space x,y.  The
//		GL_PROJECTION matrix changes all world-space coordinates of each point
//		so they are measured using those camera coords.
//			It is dangerous to think of 'GL_PROJECTION as the 'camera-
//		position-setting' matrix, because you are probably thinking of the 
//		INVERSE of GL_PROJECTION.  Remember, the camera is at the origin of
//		eye space; if we want to find the camera position in world space, we
//		must transform that eye-space origin BACKWARDS through GL_PROJECTION
//		to get back to world-space coordinates.
//
//	OPERATION:
//	  Draws a background on 'world-space'
//	  Draws a paper plane on 'model-space' 
//		--MOUSE left-click/drag applies x,y glRotation() to GL_PROJECTION 
//		--MOUSE right-click/drag applies x,y glRotation() to GL_MODELVIEW
//		--ARROW keys applies x,y glTranslate() to GL_MODELVIEW
//		-- 'R' key to reset GL_MODELVIEW matrix to initial values.
//		-- 'r' key to reset GL_PROJECTION matrix to initial values.
//		-- 'm' key to enlarge the object.
//		-- 'n' key to shrink the object.
//		-- 'Q' key to quit.
//
//	To compile this under Microsoft Visual Studio (VC++ 6.0 or later) create
//		a new Project, Win32 Console Application, and make an 'empty' project.
//		Then add this file as 'source file', be sure you have GLUT installed on
//		your machine.
//
//  If you don't have it, search the web for 'GLUT' --Marc Kilgard's
//		elegant little library that makes OpenGL platform-independent.  GLUT
//		uses 'callback' functions--you tell GLUT the names of your functions,
//		then it calls them whenever users do something that requires a response
//		from your program; for example, they moved the mouse, they resized a
//		window or uncovered it so that it must be re-drawn.
//
//  for CS 351, Northwestern University, Jack Tumblin, jet@cs.northwestern.edu
//
//	12/11/2004 - J. Tumblin--Created.
//  10/11/2005 - Modified by Seunghoon Kim for CS351 Project A.
//	
//==============================================================================
#include <math.h>							// for sin(), cos(), tan(), etc.
#include <stdlib.h>							// for all non-core C routines.
#include <stdio.h>							// for printf(), scanf(), etc.
#include <iostream>							// for cout, cin, etc.
#include <assert.h>							// for error checking by ASSERT().
#include <glut.h>							// Mark Kilgard's GLUT library.
											// (Error here? be sure you have
											// installed GLUT library
											// What's that? ask Google...
using namespace std;

#define JT_TITLE	"CS351 Project A"	// Display window's title bar:
#define JT_WIDTH	512						// window size in pixels
#define JT_HEIGHT	512
#define JT_XPOS		256						// initial window position
#define JT_YPOS		256
#define JT_ZNEAR	1.0						// near, far clipping planes for
#define JT_ZFAR		1000.0					// a 3D camera.

//====================
//
//	Function Prototypes  (these belong in a '.h' file if we have one)
//	
//====================

void glut_init(int *argc, char **argv);	// GLUT initialization
void ogl_init();						// OpenGL initialization

							// GLUT callback functions. Called when:
void display(void);						// GLUT says re-draw the display window.
void reshape(int w, int h);				// GLUT says window was re-sized to w,h
void keyboard(unsigned char key, int x, int y);	//GLUT says user pressed a key
void keySpecial(int key, int x, int y);	// GLUT says user pressed a 'special'key
void mouseMove(int xpos,int ypos);		// GLUT says user moved the mouse to..
void mouseClik(int,int,int,int);		// GLUT says user click/dragged mouse to
//void idle(void);

class jt_transRot
//==============================================================================
// Record / accumulate offset amounts and rotation amounts from mouse & keyboard
{
public:
double	x_pos, y_pos, z_pos;	// cumulative position offset
double	x_rot, y_rot, z_rot;	// cumulative rotation on x,y,z axes
double  x_scale, y_scale, z_scale;	// scale for the object
double  matrix[4][4];
double  resulting_vector[4];
int		isDragging;				// TRUE if user is holding down the mouse button
								// that affects our value(s); else FALSE.
int m_x,m_y;					// last mouse-dragging position.

~jt_transRot(void);				// default destructor
jt_transRot(void);				// default constructor
void reset(void);				// reset everything to zero.
void applyMatrix(void);			// apply translations, rotations to openGL.
};

//===================
//
// GLOBAL VARIABLES (bad idea!)
//
//====================
jt_transRot setModel;			// Changes to initial GL_MODELVIEW matrix
jt_transRot setProj;			// Changes to initial GL_PROJECTION matrix

int main(int argc, char** argv)
//------------------------------------------------------------------------------
{
	glut_init(&argc, argv);				// First initialize GLUT,
	ogl_init();							// Then initialize any non-default 
										// states we want in openGL,

	glutMainLoop();
	// Then give program control to GLUT.  This is an infinite loop, and from
	// within it GLUT will call the 'callback' functions below as needed.
	return 0;							// orderly exit.
}

//=====================
//
//  Other Function Bodies
//
//=====================

void glut_init(int *argc, char **argv)
//------------------------------------------------------------------------------
// A handy place to put all the GLUT library initial settings; note that we
// 'registered' all the function names for the callbacks we want GLUT to use.
{
	
	glutInit(argc, argv);				// GLUT's own internal initializations.

							// single buffered display, 
							//  RGB color model, use Z-buffering (depth buffer)
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(JT_WIDTH, JT_HEIGHT);	// set display-window size
	glutInitWindowPosition(JT_XPOS, JT_XPOS);	// position,
	glutCreateWindow(JT_TITLE);					// then create it.

	// Register GLUT Callback function names. (these names aren't very creative)
	glutDisplayFunc(display);			// 'display'  callback:  display();
	glutKeyboardFunc(keyboard);			// 'keyboard' callback:  keyboard(); 
	glutSpecialFunc(keySpecial);		// 'special'keys callback: keyspecial()
	glutReshapeFunc(reshape);			// 'reshape'  callback:  reshape();
//	glutIdleFunc(idle);					// 'idle'	  callback:  idle(); 

	glutMouseFunc(mouseClik);			// callbacks for mouse click, move
	glutMotionFunc(mouseMove);		
}


void ogl_init()
//------------------------------------------------------------------------------
// A handy place to put all the OpenGL initial settings-- remember, you only 
// to change things if you don't like openGL's default settings.
{
	glClearColor(0.7, 0.9, 1.0, 0.0);	// Display-screen-clearing color;
										// acts as 'background color'
	glColor3f(1.0, 1.0, 1.0);			// Select current color  for drawing
	glShadeModel(GL_FLAT);				// Choose 'flat shading' model  
//	glDisable(GL_LIGHTING);				// No lighting needed  
}


void reshape(int w, int h)
//------------------------------------------------------------------------------
// GLUT 'reshape' Callback. Called when user resizes the window from its current
// shape to one with width w, height h.
// We usually initialize openGL's 'GL_PROJECTION' matrix here.
{
	// set size of viewport to window size
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);

	// Set the projection matrix  
	glMatrixMode(GL_PROJECTION);		// Select the Projection matrix,

//***CHOOSE A CAMERA:***

//	gluOrtho2D(0, w, 0, h);	// orthographic projection(left,right,bottom,top): 
							// using (0,w,0,h) maps x,y world space to screen
							// space in pixel units.
//**OR**

/*
	glLoadIdentity();		// (Clear out any previous camera settings)
	gluPerspective(45.0,	// Set camera's vertical field-of-view  (y-axis)
							// measured in degrees, and set the
		(double)w/(double)h,// aspect ratio of displayed image (width/height),
		JT_ZNEAR,JT_ZFAR);	// and set near, far clipping planes.
							// if GL_PERSPECTIVE matrix was identity before we
							// called gluPerspective, then we're at the world
							// space origin, but looking in the (world space)
							// -Z direction. 
							// (if current matrix is NOT identity, then the
							// current matrix M is pre-multiplied by the matrix
							// 'T' spec'd by gluPerspective: new matrix is MT).
							//
	// REMEMBER, all vertices are first multipled by the GL_MODELVIEW matrix,
	// and then by the GL_PROJECTION matrix before the 'viewport' maps them
	// to the display window.
	// REMEMBER when you call glTranslate() or glRotate() in openGL, existing 
	// GL_PROJECTION or GL_MODELVIEW is ***PRE_MULTIPLIED*** by the specified
	// translate or rotate matrix to make the GL_PROJECTION or GL_MODELVIEW
	// matrix!  This is *NOT* intuitive!
	//		THUS, if we call glTranslatef(0,0,-10), then the world-space origin
	// is transformed to (0,0,-10) *BEFORE* we apply the camera 
	// matrix that turns it into a picture:
		glTranslatef(0.0f, -2.0f, -10.0f);
	// This gives you the same picture you'd get if you'd translated the
	// camera to the world-space location (0,+2,+10). Confusing, isn't it?!
	// Here's a good way to think of it; 
	//		1) the camera is at the origin of 'eye' space, and looking in the 
	//			-Z direction in 'eye' space.  
	//		2) The glTranslate(0,0,-10) above converts world space coords to 
	//			eye-space coordinates. 
	// The INVERSE transform (e.g. glTranslate(0,0,+10) converts eye-space
	// coords to world space.  The camera is always the origin of eye-space; 
	// if we transform the eye-space origin to world space, we find the camera's
	// world-space position is 0,0,+10.
		glRotatef(30.0f, 0.0f, 1.0f, 0.0f);
	//		Similarly, if we next call glRotationf(30.0,0,1,0) (e.g. rotate by
	// 30 degrees around the y axis) the current contents of the GL_PROJECTION
	// matrix is again pre-multiplied by the new rotation matrix we made.  Any 
	// point in world space is rotated  (about the world-space origin) first, 
	// then translated to make eye-space coordinates (where the camera is at
	// the origin and looking down the -Z axis).  
	// Just as before, the INVERSE transform (eye-space-to-world space) tells
	// us the camera position in world space. Take the origin of eye space
	// (e.g. the camera position) translate(0,0,+10) so now camera is at 0,0,10
	// and still looking in -Z direction towards origin. Next, rotate about
	// the Y axis by -30 degrees, causing the camera to swing around from the
	// Z axis towards the -X axis. 
*/

//**OR**

	glLoadIdentity();			// (Clear out any previous camera settings)
	gluPerspective(				// Set camera's internal parameters: 
		45.0,					// vertical (y-axis) field-of-view in degrees,
		(double)w/(double)h,	// display image aspect ratio (width/height),
		JT_ZNEAR,JT_ZFAR);		// near,far clipping planes for camera depth.

	gluLookAt(-5.0, 2.0, 8.66,	// VRP: eyepoint x,y,z position in world space.
			   0.0, 0.0, 0.0,	// 'look-at' point--we're looking at origin.
								// (VPN = look-at-point - VRP)
			   0.0, 1.0, 0.0);	// VUP: view 'up' vector; set 'y' as up...
	//*** SURPRISE****
	// the matrix made by gluLookAt() *POST-MULTIPLIES* the current matrix,
	// unlike the glRotate() and glTranslate() functions.

	// Puzzle: What would happen now if you called 'glTranslate(0,0,-10)?
	// can you explain what happens if you then call 'glRotate(30f,0,1,0)?

	// Initialize the modelview matrix to do nothing.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();					// Set it to 'do nothing'.
}

void display(void)
//------------------------------------------------------------------------------
// GLUT 'display' Callback.  GLUT calls this fcn when it needs you to redraw 
// the dislay window's contents.  Your program should never call 'display()',
// because it will confuse GLUT--instead, call glutPostRedisplay() if you need
// to trigger a redrawing of the screen.
{
	// Clear the frame-buffer  
	glClear(GL_COLOR_BUFFER_BIT);

// =============================================================================
// START DRAWING CODE HERE 
// =============================================================================
	glMatrixMode(GL_PROJECTION);	// select projection matrix,
	glPushMatrix();					// save current version, then
	setProj.applyMatrix();			// apply results of mouse, keyboard

	// Draw model-space axes:
	glMatrixMode(GL_MODELVIEW);		// select the modelview matrix,
	glPushMatrix();					// save current version, then
	setModel.applyMatrix();			// apply results of mouse, keyboard

	glBegin(GL_TRIANGLES);
		glColor3f (0.9, 0.9, 0.9);		// inner right body
		glVertex3f(0.0*setModel.x_scale, 0.0*setModel.y_scale, 0.0*setModel.z_scale);
		glVertex3f(0.3*setModel.x_scale, 0.5*setModel.y_scale, 0.0*setModel.z_scale);
		glVertex3f(0.0*setModel.x_scale, 0.7*setModel.y_scale, -1.0*setModel.z_scale);
	glEnd();

	glBegin(GL_TRIANGLES);
		glColor3f (0.9, 0.9, 0.9);		// inner left body
		glVertex3f(0.0*setModel.x_scale, 0.0*setModel.y_scale, 0.0*setModel.z_scale);
		glVertex3f(-0.3*setModel.x_scale, 0.5*setModel.y_scale, 0.0*setModel.z_scale);
		glVertex3f(0.0*setModel.x_scale, 0.7*setModel.y_scale, -1.0*setModel.z_scale);
	glEnd();

	glBegin(GL_TRIANGLES);
		glColor3f (1.0, 1.0, 1.0);		// outer right wing
		glVertex3f(0.8*setModel.x_scale, 0.3*setModel.y_scale, 0.2*setModel.z_scale);
		glVertex3f(0.3*setModel.x_scale, 0.5*setModel.y_scale, 0.0*setModel.z_scale);
		glVertex3f(0.0*setModel.x_scale, 0.7*setModel.y_scale, -1.0*setModel.z_scale);
	glEnd();

	glBegin(GL_TRIANGLES);
		glColor3f (1.0, 1.0, 1.0);		// outer left wing
		glVertex3f(-0.8*setModel.x_scale, 0.3*setModel.y_scale, 0.2*setModel.z_scale);
		glVertex3f(-0.3*setModel.x_scale, 0.5*setModel.y_scale, 0.0*setModel.z_scale);
		glVertex3f(0.0*setModel.x_scale, 0.7*setModel.y_scale, -1.0*setModel.z_scale);
	glEnd();


	glBegin(GL_LINES);				// draw axes in model-space.

		glColor3f ( 0.8, 0.8, 0.8);	// inner vertex right
		glVertex3f( 0.3*setModel.x_scale, 0.5*setModel.y_scale, 0.0*setModel.z_scale);
		glVertex3f( 0.0*setModel.x_scale, 0.7*setModel.y_scale, -1.0*setModel.z_scale);

		glColor3f ( 0.8, 0.8, 0.8);	// inner vertex left
		glVertex3f( -0.3*setModel.x_scale, 0.5*setModel.y_scale, 0.0*setModel.z_scale);
		glVertex3f( 0.0*setModel.x_scale, 0.7*setModel.y_scale, -1.0*setModel.z_scale);

		glColor3f ( 0.8, 0.8, 0.8);	// backbone
		glVertex3f( 0.0*setModel.x_scale, 0.0*setModel.y_scale, 0.0*setModel.z_scale);
		glVertex3f( 0.0*setModel.x_scale, 0.7*setModel.y_scale, -1.0*setModel.z_scale);
		
	glEnd();

	// Draw axes in world-space:	
	glLoadIdentity();			// wipe out current GL_MODELVIEW matrix so that
								// model-space vertices become world-space
								// vertices without change.
	
	glPopMatrix();				// restore original MODELVIEW matrix.
	glMatrixMode(GL_PROJECTION);// Restore the original GL_PROJECTION matrix
	glPopMatrix();
	// ================================================================================
	// END DRAWING CODE HERE 
	// ================================================================================
	
	cout << "Screen ReDrawn" << endl;
	glFlush();
}                                                                                                                                                                                 

void keyboard(unsigned char key, int x, int y)
//------------------------------------------------------------------------------
// GLUT 'keyboard' Callback.  User pressed an alphanumeric keyboard key.
// ('special' keys such as return, function keys, arrow keys? keyboardspecial)
{
	double cosine = cos(3.14/12);		// cosine and sine values of 15 degree
	double sine = sin(3.14/12);
	double orig_xpos = setProj.x_pos;
	double orig_ypos = setProj.y_pos;
	switch(key) {
		case 27: // Esc  
		case 'Q':
		case 'q':
			exit(0);		// Quit application  
			break;
		case 'm':				// enlarge the object
			setModel.x_scale *= 1.1;
			setModel.y_scale *= 1.1;
			setModel.z_scale *= 1.1;
			break;
		case 'n':					// shrink the object
			setModel.x_scale *= 0.9;
			setModel.y_scale *= 0.9;
			setModel.z_scale *= 0.9;
			break;
		case 'r':
			setProj.reset();
			break;
		case 'R':
			setModel.reset();
			break;
		default:
			printf("unknown key.  Try arrow keys, r, R, or q");
			break;
	}
	// We might have changed something. Force a re-display  
	glutPostRedisplay();
}

void keySpecial(int key, int x, int y)
//------------------------------------------------------------------------------
// GLUT 'special' Callback.  User pressed an non-alphanumeric keyboard key, such
// as function keys, arrow keys, etc.
{
static double x_pos, y_pos, z_pos;

	switch(key)	
	{
		case GLUT_KEY_UP:		// up arrow key
			setModel.z_pos -= 0.1;
			break;
		case GLUT_KEY_DOWN:		// dn arrow key
			setModel.z_pos += 0.1;
			break;
		case GLUT_KEY_LEFT:		// left arrow key
			setModel.x_pos -= 0.1;
			break;
		case GLUT_KEY_RIGHT:	// right arrow key
			setModel.x_pos += 0.1;
			break;
		default:
			break;
	}
	printf("key=%d, setModel.x_pos=%f, setModel.z_pos=%f\n",
							key,setModel.x_pos,setModel.z_pos);
	// We might have changed something. Force a re-display  
	glutPostRedisplay();
}

void mouseClik(int buttonID,int upDown,int xpos,int ypos)
//------------------------------------------------------------------------------
// GLUT 'mouse' Callback.  User caused a click/unclick event with the mouse:
//     buttonID== 0 for left mouse button,
//			  (== 1 for middle mouse button?)
//			   == 2 for right mouse button;
//		upDown == 0 if mouse button was pressed down,
//			   == 1 if mouse button released.
//		xpos,ypos == position of mouse cursor, in pixel units within the window.
// *CAREFUL!* Microsoft puts origin at UPPER LEFT corner of the window.
{
	if(buttonID==0)				// if left mouse button,
	{
		if(upDown==0)			// on mouse press,
		{
			setProj.isDragging = 1;	// get set to record GL_PROJECTION changes.
			setProj.m_x = xpos;		// Dragging begins here.
			setProj.m_y = ypos;
		}
		else setProj.isDragging = 0;
	}
	else if(buttonID==2)		// if right mouse button,
	{
		if(upDown==0)
		{
			setModel.isDragging = 1;// get set to record GL_MODELVIEW changes.
			setModel.m_x = xpos;	// Dragging begins here.
			setModel.m_y = ypos;
		}
		else setModel.isDragging = 0;
	}
	else						// something else.
	{
		setProj.isDragging  = 0;	// default; DON'T change GL_PROJECTION
		setModel.isDragging = 0;	//					or  GL_MODELVIEW
	}

	printf("clik: buttonID=%d, upDown=%d, xpos=%d, ypos%d\n",
										buttonID,upDown,xpos,ypos);
}

void mouseMove(int xpos,int ypos)
//------------------------------------------------------------------------------
// GLUT 'move' Callback.  User moved the mouse while pressing 1 or more of the
// mouse buttons.  xpos,ypos is the MS-Windows position of the mouse cursor in
// pixel units within the window.
// CAREFUL! MSoft puts origin at UPPER LEFT corner pixel of the window!
{
#define JT_INCR 1.0					// Degrees rotation per pixel of mouse move

	if(setModel.isDragging==1)			// if we're dragging the left mouse,
	{								// increment the x,y rotation amounts.
		double xdiff = JT_INCR*(xpos - setModel.m_x);
		double ydiff = JT_INCR*(ypos - setModel.m_y);
		printf("XDiff, YDiff = %d, %d\n", xdiff,ydiff);	// print what we did.
		setModel.m_x = xpos;
		setModel.m_y = ypos;

		double degree = atan(xdiff/ydiff);
		printf("Degree = %d\n", degree);	// print what we did.
		setModel.matrix[0][0] = cos(degree);
		setModel.matrix[0][1] = -sin(degree);
		setModel.matrix[0][2] = 0;
		setModel.matrix[1][0] = sin(degree);
		setModel.matrix[1][1] = cos(degree);
		setModel.matrix[1][2] = 0;
		setModel.matrix[2][0] = 0;
		setModel.matrix[2][1] = 0;
		setModel.matrix[2][2] = 1;

		
            setModel.x_rot = setModel.matrix[0][0]*xpos + setModel.matrix[0][1]*ypos;
            setModel.y_rot = setModel.matrix[1][0]*xpos + setModel.matrix[1][1]*ypos;
		

		printf("move %d, %d\n", xpos,ypos);	// print what we did.
	}
	if(setProj.isDragging==1)		// if we're dragging theright mouse,
	{								// increment the x,y rotation amounts.
		setProj.x_rot += JT_INCR*(xpos - setProj.m_x);
		setProj.y_rot += JT_INCR*(ypos - setProj.m_y);
		setProj.m_x = xpos;
		setProj.m_y = ypos;
		printf("move %d, %d\n", xpos,ypos);	// print what we did.
	}

	// We might have changed something. Force a re-display  
	glutPostRedisplay();

#undef JT_INCR
}

/*
void idle(void)
//------------------------------------------------------------------------------
// GLUT 'idle' Callback. Called when OS has nothing else to do; a 'clock tick'.  
// Use 'idle' *ONLY IF* your program does anything that needs continual updates, even 
// when users are not pressing keys, then put code to do the updates here.
// If you need to redraw the screen after your update, don't forget to call
// glutPostRedisplay() too.
//			*** A COMMON MISTAKE TO AVOID: ***
// 'idle()' gets called VERY OFTEN.  If you register 'idle()' and leave the idle
// function empty, GLUT will waste most/all CPU time not otherwise used on
// useless calls to idle().  If idle() contains only glutPostRedisplay(), you
// will force GLUT to redraw the screen as often as possible--even if the 
// contents of the screen has not changed.  If your program ONLY changes screen 
// contents when user moves,clicks, or drags the mouse, presses a key, etc.,
// then you don't need idle() at all! Instead, call glutPostRedisplay() at the 
// end of each of the GLUT callbacks that change the screen contents.  
// Then you'll update the screen only when there is something new to show on it.
{

}
*/

//==============================================================================
// jt_transRot function bodies:

jt_transRot::~jt_transRot(void)
//------------------------------------------------------------------------------
// Default destructor
{
}

jt_transRot::jt_transRot(void)
//------------------------------------------------------------------------------
// Default constructor
{
	reset();						// set all values to zero.
}

void jt_transRot::reset(void)
//------------------------------------------------------------------------------
// Set all values to zero.
{
	x_pos = 0.0; y_pos = 0.0; z_pos = 0.0;
	x_rot = 0.0; y_rot = 0.0; z_rot = 0.0;
	x_scale = 1.0; y_scale = 1.0; z_scale = 1.0;
}

void jt_transRot::applyMatrix(void)
//------------------------------------------------------------------------------
// Apply translations, then rotations.  (Note OpenGL pre-multiplies matrices,
// so commands appear to be in reverse order).
{
	glRotated(z_rot, 0.0, 0.0, 1.0);
	glRotated(y_rot, 0.0, 1.0, 0.0);
	glRotated(x_rot, 1.0, 0.0, 0.0);
	glTranslated(x_pos, y_pos, z_pos);
}

