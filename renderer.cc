#include <iostream>
#include <stdio.h>
#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>

#include "renderer.h"
#include "world.h"

Renderer::Renderer(World *w): world(w) {

	init_shaders();
	init_geometry();
	init_player();

	return;
}

Renderer::~Renderer() {
	return;
}

void Renderer::draw() {
	Rect *viewport = world->get_viewport();
	float matrix[] = {
		1, 0, 0,
		0, 1, 0,
		0, 0, 1};
	// TODO: Kill this, just quieting a compiler error
	matrix[1] = viewport->coords[0] * 0;

	// Get the world viewport exactly over the screen viewport
	glUniformMatrix3fv(viewport_uniform, 1, GL_FALSE, matrix);

	// Fire off triangle lists, etc.
	draw_stage();
	draw_player();
	draw_rope();
	
	return;
}

void Renderer::draw_stage() {
	glBindVertexArray(geometry_array);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Renderer::init_geometry() {
	// Pull the geometry out of world and stick it in vram
	GLfloat vertices[] = {
		-1, 0,
		 1, 0,
		 0, 1};
	glGenVertexArrays(1, &geometry_array);
	glBindVertexArray(geometry_array);

	glGenBuffers(1, &geometry_object);
	glBindBuffer(GL_ARRAY_BUFFER, geometry_object);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// hard coding vertices as 0... not likely to be changed in here
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	return;
}

void Renderer::draw_player() {
	// Update the player mesh first
	const Player &p = world->get_player();
	const Vec2 &pos = p.get_position();
	
	glBindVertexArray(player_array);

	Vec2 *mmap = (Vec2*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	*mmap = pos;
	mmap->y = -0.5; //temporary, so stuff doesn't get pushed off-screen
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glDrawArrays(GL_POINTS, 0, 1);
	return;
}

void Renderer::init_player() {
	// Load up the mesh/skeleton/thing
	GLfloat vertices[] = {
		1.0, 0};
	glGenVertexArrays(1, &player_array);
	glBindVertexArray(player_array);

	glGenBuffers(1, &player_object);
	glBindBuffer(GL_ARRAY_BUFFER, player_object);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	GLenum error = glGetError();
	while (error != GL_NO_ERROR) {
		std::cout << "Error: " << error << "\n";
		error = glGetError();
	}
	return;
}

void Renderer::draw_rope() {
	return;
}

void Renderer::init_shaders() {

	vshader = glCreateShader(GL_VERTEX_SHADER);
	load_shader(vshader, (char *)"shader.vp");

	fshader = glCreateShader(GL_FRAGMENT_SHADER);
	load_shader(fshader, (char *)"shader.fp");

	shaderprogram = glCreateProgram();
	glAttachShader(shaderprogram, vshader);
	glAttachShader(shaderprogram, fshader);
	glLinkProgram(shaderprogram);
	glUseProgram(shaderprogram);

	GLint linkstatus;
	glGetProgramiv(shaderprogram, GL_LINK_STATUS, &linkstatus);
	if (linkstatus != GL_TRUE) {
		printf("Link failed!\n");
		exit(-1);
	}

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	viewport_uniform = glGetUniformLocation(shaderprogram, "Viewport");
}

void Renderer::load_shader(GLuint shader, char *filename) {

	FILE *f = fopen(filename, "r");
	fseek(f, 0, SEEK_END);
	const int size = ftell(f);
	rewind(f);

	//GLchar *source = (GLchar *)calloc(size + 1, 1);
	GLchar *source = new GLchar[size+1];
	fread((char *)source, size, 1, f);

	glShaderSource(shader, 1, (const GLchar **)&source, &size);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		GLint logsize;
		GLint logsize_ret;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
		GLchar *log = new GLchar[size]; //(GLchar *)malloc(logsize);
		glGetShaderInfoLog(shader, logsize, &logsize_ret, log);
		printf("Shader %s compilation failed!\n%s\n", filename, log);
		//free(log);
		delete[] log;
		exit(-1);
	}

	//free((void *)source);
	delete[] source;
	fclose(f);

	return;
}
