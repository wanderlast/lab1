//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#define WINDOW_WIDTH  900
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 6000
#define GRAVITY 0.1
#define MAX_BOXES 5

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape box[MAX_BOXES];
	Shape circle;
	Particle particle[MAX_PARTICLES];
	int n;
	int lastMousex, lastMousey;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
//void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;

	//make this work iteratively -- you can do it in a loop making them descend downwards
	//declare a box shape
	for (int i = 0; i < MAX_BOXES; i++)
	{
	    game.box[i].width = 100;
	    game.box[i].height = 20;
	    game.box[i].center.x = 120 + (i * 120);
	    game.box[i].center.y = 500 - (i * 70);
	}
	
	//declare a circle shape
	game.circle.center.x = 900;
	game.circle.center.y = 0;
	game.circle.radius = 180;

	//start animation
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			//check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
							ButtonPress | ButtonReleaseMask |
							PointerMotionMask |
							StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
					InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
}

#define rnd() (float)rand() / (float)RAND_MAX

void makeParticle(Game *game, int x, int y) {
	if (game->n >= MAX_PARTICLES)
		return;
	std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = 1 - rnd()*0.8;
	p->velocity.x = 1.0 + rnd()*0.5;
	game->n++;
}

// void check_mouse(XEvent *e, Game *game)
// {
// 	static int savex = 0;
// 	static int savey = 0;
// 	//static int n = 0;
// 
// 	if (e->type == ButtonRelease) {
// 		return;
// 	}
// 	if (e->type == ButtonPress) {
// 		if (e->xbutton.button==1) {
// 			//Left button was pressed
// 			int y = WINDOW_HEIGHT - e->xbutton.y;
// 			for (int i=0; i < 10; i++)
// 				makeParticle(game, e->xbutton.x, y);
// 			return;
// 		}
// 		if (e->xbutton.button==3) {
// 			//Right button was pressed
// 			return;
// 		}
// 	}
// 	//Did the mouse move?
// 	if (savex != e->xbutton.x || savey != e->xbutton.y) {
// 		savex = e->xbutton.x;
// 		savey = e->xbutton.y;
// 		int y = WINDOW_HEIGHT - e->xbutton.y;
// 		for (int i=0; i < 10; i++)
// 			makeParticle(game, e->xbutton.x, y);
// 		//if (++n < 10)
// 		//return;
// 		game->lastMousex = e-> xbutton.x;
// 		game->lastMousey = y;
// 		
// 	}
// }

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape){
			return 1;
		}
		//You may check other keys here.
		if (key == XK_b){
			//turn on bubbler
			for (int i=0; i < 20; i++){
				makeParticle(game, 120, 550);
			}
		}

	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;
	
	for (int i=0; i < game->n; i++){
	  p = &game->particle[i];
	  p->s.center.x += p->velocity.x;
	  p->s.center.y += p->velocity.y;
	  p->velocity.y -= GRAVITY;
	
	  //check for collision with shapes...
	  //box collision
	  for (int j = 0; j < MAX_BOXES; j++){
		Shape *s = &game->box[j];
		if (p->s.center.y < s->center.y + s->height &&
		    p->s.center.y > s->center.y - s->height &&
		    p->s.center.x >= s->center.x - s->width &&
		    p->s.center.x <= s->center.x + s->width){
			p->velocity.y *= -0.2;
			p->s.center.y = s->center.y + s->height + 0.01;
	      }
	  }
	  
	  //circle collision
	  for
	  
	  //check for off-screen
	  if (p->s.center.y < 0.0 || p->s.center.y > WINDOW_HEIGHT) {
		//sstd::cout << "off screen" << std::endl;
		memcpy(&game->particle[i], &game->particle[game->n-1], sizeof(Particle));
		game->n--;
	  }

	}
}

void render(Game *game)
{
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...	
	
	//draw circle
	//do this only if our first time through the loop
	static int firsttime = 1;
	static int verts[60][2]; //10 vertices, 2 dimensions perspective
	static int n = 60;
	glColor3ub(70,120,100);
	if (firsttime){
		float angle = 0.0;
		float inc = (3.14159 * 2.0) / (float)n;
		
		for(int i = 0; i < n; i++){
			verts[i][0] = cos(angle) * game->circle.radius + game->circle.center.x;
			verts[i][1] = sin(angle) * game->circle.radius + game->circle.center.y;
			angle += inc;
		}
		firsttime = 0;	  
	}
	
	glPushMatrix();
	glBegin(GL_TRIANGLE_FAN);
		for(int i = 0; i < n; i++){
			glVertex2i(verts[i][0], verts[i][1]);
		}

	glEnd();
	glPopMatrix();
	
	//draw box
	Shape *s;
	glColor3ub(90,140,90);
	for (int j = 0; j < MAX_BOXES; j++){
		s = &game->box[j];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h); 	
			glVertex2i( w,-h);
		glEnd();
		glPopMatrix();
	}

	//draw all particles here
	for (int i=0; i < game->n; i++){
		glPushMatrix();
		glColor3ub(150,160,220);
		Vec *c = &game->particle[i].s.center;
		w = 2;
		h = 2;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
}
