#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "renderer.h"
#include "world.h"
#include "stage.h"

// Tradition? Even though my display is 1080p...
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

// global so the GLUT callbacks can get to them
Renderer *renderer;
World *world;
Player *player;
Stage *stage;

enum Keys {
	LEFT,
	RIGHT,
	UP,
	DOWN,
	JUMP,
	FIRE,
	RETRACT,
	LAST_KEY
};
bool key_states[LAST_KEY];
bool key_last_state[LAST_KEY];
int mouse_pos[2];
int window_size[2] = {WINDOW_WIDTH, WINDOW_HEIGHT};

//Do all the ugly GLUT stuff here because the callbacks don't allow passing around arguments

void render() {
	glClear(GL_COLOR_BUFFER_BIT);

	static int last_msec = 0;
	int msec = glutGet(GLUT_ELAPSED_TIME);
	// Stupid busy loop to make sure updates actually work... forced by timer resolution and other junk
	while (msec == last_msec) {
		msec = glutGet(GLUT_ELAPSED_TIME);
	}
	float timestep = (msec - last_msec) / 1000.0f;
	last_msec = msec;
	// For safety, cap the min framerate - don't break physics assumptions
	if (timestep > 0.05) { // 20fps
		timestep = 0.05;
	}

	// Input crunching
	// Biased right and up
	float x = 0, y = 0;
	if (key_states[LEFT]) {
		x = -1.0f;
	} else if (key_states[RIGHT]) {
		x = 1.0f;
	}
	if (key_states[DOWN]) {
		y = -1.0f;
	} else if (key_states[UP]) {
		y = 1.0f;
	}

	// TODO: edge trigger, not level trigger
	if (key_states[FIRE] && not key_last_state[FIRE]) {
		// Get it into world coordinates
		float screen_coords[2];
		screen_coords[0] = (float)mouse_pos[0] / window_size[0];
		// Y is inverted in screen -> world
		screen_coords[1] = (float)(window_size[1] - mouse_pos[1]) / window_size[1];
		const Rect &viewport = world->get_viewport();
		float world_coords[2];
		world_coords[0] = viewport.x1 + (viewport.x2 - viewport.x1) * screen_coords[0];
		world_coords[1] = viewport.y1 + (viewport.y2 - viewport.y1) * screen_coords[1];
		player->trigger(world_coords[0], world_coords[1]);
	}
	key_last_state[FIRE] = key_states[FIRE];

	player->control(x, y);
	player->retract(key_states[RETRACT]);
	player->jump(key_states[JUMP]);

	world->update(timestep);

	renderer->draw();

	glutSwapBuffers();
	glutPostRedisplay();
	return;
}

Keys key_map(int key) {
	switch(key) {
		case 'a':
			return LEFT;
		case 'e':
		case 'd':
			return RIGHT;
		case 'o':
		case 's':
			return DOWN;
		case ',':
		case 'w':
			return UP;
		case ' ':
			return JUMP;
	}
	return LAST_KEY;
}

void keyboard(unsigned char key, int x, int y) {
	int keysym = key_map(key);
	if (keysym == LAST_KEY) {
		return;
	}
	key_states[keysym] = 1;
	return;
}

// This logic does lead to glitches with the multiple mapping
// but if you're using Dvorak and Qwerty simultaneously, not my problem!
void keyboardup(unsigned char key, int x, int y) {
	int keysym = key_map(key);
	if (keysym == LAST_KEY) {
		return;
	}
	key_states[keysym] = 0;
	return;
}
// glutSpecialFunc has a different signature
void keyboard2(int key, int x, int y) {
	return;
}
void keyboard2up(int key, int x, int y) {
	return;
}

void mousebutton(int button, int state, int x, int y) {
	int key;
	switch(button) {
		case GLUT_LEFT_BUTTON:
			key = FIRE;
			break;
		case GLUT_RIGHT_BUTTON:
			key = RETRACT;
			break;
		default:
			return;
	}
	// state should be GLUT_DOWN or GLUT_UP... they're probably 1 and 0 anyway
	key_states[key] = state == GLUT_DOWN ? 1 : 0;
	mouse_pos[0] = x;
	mouse_pos[1] = y;
	return;
}

void mousemotion(int x, int y) {
	return;
}

void resize(int width, int height) {
	world->set_aspect_ratio((float)width/(float)height);
	window_size[0] = width;
	window_size[1] = height;
	std::cout << "New width: " << width << " New height: " << height << "\n";
}

// yes, this breaks the semantics of argc and argv...
void osd_init(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitContextVersion(2, 1); // Intel 4500

	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE); // ???

	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	int windowHandle = glutCreateWindow("aoeu");
	if (windowHandle < 0) {
		fprintf(stderr, "Couldn't create window\n");
		exit(EXIT_FAILURE);
	}

	glewExperimental = GL_TRUE;
	GLenum glewResult = glewInit();

	if (glewResult != GLEW_OK) {
		fprintf(stderr, "GLEW Init failure: %s.\n", glewGetErrorString(glewResult));
		exit(EXIT_FAILURE);
	}

	glutDisplayFunc(render);
	glutMouseFunc(mousebutton);
	glutMotionFunc(mousemotion);
	glutPassiveMotionFunc(mousemotion);
	glutKeyboardFunc(keyboard);
//	glutSpecialFunc(keyboard2);
	glutKeyboardUpFunc(keyboardup); //yay freeglut
//	glutSpecialUpFunc(keyboard2up);
	glutReshapeFunc(resize);

}

int main(int argc, char *argv[]) {
	osd_init(argc, argv);

	world = new World();
	stage = new Stage("test1.stg");
	world->set_stage(stage);
	const Vec2 &player_origin = stage->get_origin();
	player = new Player(player_origin.x, player_origin.y, *world);
	world->set_player(player);
	// populate world as necessary
	renderer = new Renderer(world);

	glutMainLoop();
	return EXIT_SUCCESS;
}

