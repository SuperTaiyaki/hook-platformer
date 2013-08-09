#include <iostream>
#include <stdio.h>
#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>

#include "renderer.h"
#include "world.h"

Renderer::Renderer(World *w): world(w) {

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	init_shaders();
	init_geometry();
	init_player();
	init_rope();

	return;
}

Renderer::~Renderer() {
	return;
}

void Renderer::draw() {
	const Rect &viewport = world->get_viewport();
	float matrix[] = {
		1, 0, 0,
		0, 1, 0,
		0, 0, 1};
	float new_origin[2];

	// Get the world viewport exactly over the screen viewport
	new_origin[0] = viewport.x1 + (viewport.x2 - viewport.x1) / 2.0f;
	new_origin[1] = viewport.y1 + (viewport.y2 - viewport.y1) / 2.0f;
	// Put in scaling stuff
	matrix[0] = 2.0f/(viewport.x2 - viewport.x1);
	matrix[4] = 2.0f/(viewport.y2 - viewport.y1);
	// Translation
	// Simple, so no need to run this through a proper multiplication
	// resulting numbers are in _screen space_
	matrix[6] = new_origin[0] * matrix[0] * -1.0f;
	matrix[7] = new_origin[1] * matrix[4] * -1.0f;
	//Target space is [-1, 1] on both axes

	glUniformMatrix3fv(viewport_uniform, 1, GL_FALSE, matrix);

	// Fire off triangle lists, etc.
	draw_stage();
	draw_player();
	draw_rope();
	
	return;
}

void Renderer::updateViewport(int x, int y) {
	glViewport(0, 0, x, y);
}

void Renderer::draw_stage() {
	glBindVertexArray(geometry_array);
	glUniform3f(color_uniform, 0, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, geometry_vertices);
}

void Renderer::init_geometry() {
	// Pull the geometry out of world and stick it in vram
	/*GLfloat vertices[] = {
		-100, 0,
		 100, 0,
		 0, 100}; */

	const Stage &stage = world->get_stage();
	const std::list<Rect*> &geometry = stage.get_geometry();

	// 6 vertices per box, x/y per vertex
	geometry_vertices = geometry.size() * 6*2;
	GLfloat *vertices = new GLfloat[geometry_vertices];
	int box = 0;
	for (std::list<Rect *>::const_iterator iter = geometry.begin();
		       	iter != geometry.end(); iter++) {

		// First triangle, top left, bottom left, bottom right
		// 2nd triangle top right, top left, bottom right
		/* 0,4 *----* 3
		 *     |\   |
		 *     |  \ |
		 * 1   *----* 2,5
		 */
		Rect *rect = *iter;
#define coord(vtx, y) (vertices[box*12 + 2*vtx + y*1])
/*		vertices[box*12 + 2*0 + 0] = rect.x1;
		vertices[box*12 + 2*5 + 0] = rect.x1 */
		coord(0, 0) = coord(4, 0) = coord(1, 0) = rect->x1;
		coord(3, 0) = coord(2, 0) = coord(5, 0) = rect->x2;
		coord(1, 1) = coord(2, 1) = coord(5, 1) = rect->y1;
		coord(0, 1) = coord(4, 1) = coord(3, 1) = rect->y2;
#undef coord
		box += 1;
	}

	glGenVertexArrays(1, &geometry_array);
	glBindVertexArray(geometry_array);

	glGenBuffers(1, &geometry_object);
	glBindBuffer(GL_ARRAY_BUFFER, geometry_object);
	glBufferData(GL_ARRAY_BUFFER, geometry_vertices*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	// hard coding vertices as 0... not likely to be changed in here
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	delete[] vertices;

	return;
}

void Renderer::draw_player() {
	// Update the player mesh first
	const Player &p = world->get_player();
	const Vec2 &pos = p.get_position();

	glBindVertexArray(player_array);
	glBindBuffer(GL_ARRAY_BUFFER, player_object);

	// Not using a Vec2 beacuse the vptr might make things messy
	GLfloat *mmap = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	mmap[0] = pos.x;
	mmap[1] = pos.y; 
//	Vec2 *mmap = (Vec2*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
//	*mmap = pos;
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUniform3f(color_uniform, 1.0f, 0.0f, 0.0f);

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

void Renderer::init_rope() {
	glGenVertexArrays(1, &rope_array);
	glBindVertexArray(rope_array);

	glGenBuffers(1, &rope_object);
	glBindBuffer(GL_ARRAY_BUFFER, rope_object);
	// Possible optimization: pre-allocate a decent buffer, instead of constantly resizing
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 2, NULL, GL_DYNAMIC_DRAW);
	rope_buffer_size = 2;
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

void Renderer::draw_rope() {

	const std::list<Vec2> &nodes = world->get_player().get_rope_path();
	if (nodes.size() < 2) {
		return;
	}

	glBindVertexArray(rope_array);
	glBindBuffer(GL_ARRAY_BUFFER, rope_object);
	if (nodes.size() > rope_buffer_size) {
		// Should this just double the size until it hits rope_buffer_size?
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * nodes.size(), NULL, GL_DYNAMIC_DRAW);
		rope_buffer_size = nodes.size();
	}

	GLfloat *mmap = (GLfloat *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	for (std::list<Vec2>::const_iterator iter = nodes.begin();
			iter != nodes.end(); iter++) {
		// Bulk copy would be nice...
		*mmap++ = (*iter).x;
		*mmap++ = (*iter).y;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	glUniform3f(color_uniform, 0, 0, 1);
	glDrawArrays(GL_LINE_STRIP, 0, nodes.size());
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
	color_uniform = glGetUniformLocation(shaderprogram, "Color");
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
