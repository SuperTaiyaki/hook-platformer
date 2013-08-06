#ifndef __renderer_h__
#define __renderer_h__

#include "world.h"

class Renderer {
	public:
		Renderer(World *w);
		~Renderer();

		void draw();

	private:
		World *world;

		void init_geometry();

		void draw_stage();
		void draw_player();
		void draw_rope();

		void init_shaders();
		void load_shader(GLuint shader, char *filename);

		GLuint vshader, fshader, shaderprogram, viewport_uniform;
		GLuint geometry_array, geometry_object;

};

#endif //renderer_h
