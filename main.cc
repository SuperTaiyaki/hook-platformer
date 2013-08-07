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

//Do all the ugly GLUT stuff here because the callbacks don't allow passing around arguments

void render(void) {
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
	if (timestep > 0.05) {
		timestep = 0.05;
	}

	world->update(timestep);

	renderer->draw();

	glutSwapBuffers();
	glutPostRedisplay();
	return;
}

void keyboard(unsigned char key, int x, int y) {
	//printf("Keyboard: %d %d %d\n", key, x, y);
	return;
}
// glutSpecialFunc has a different signature
void keyboard2(int key, int x, int y) {
	return;
}

void mousebutton(int button, int state, int x, int y) {
	return;
}

void mousemotion(int x, int y) {
	return;
}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitContextVersion(2, 1); // Intel 4500

	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE); // ???

	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
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
	glutSpecialFunc(keyboard2);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	world = new World();
	player = new Player(0, 10);
	world->set_player(player);
	// populate world as necessary
	renderer = new Renderer(world);

	glutMainLoop();
	return EXIT_SUCCESS;
}

