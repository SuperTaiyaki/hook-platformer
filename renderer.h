#ifndef __renderer_h__
#define __renderer_h__

#include "world.h"

class Renderer {
	public:
		Renderer(World *w);
		~Renderer();

		void draw();

		void updateViewport(int x, int y);

	private:
		World *world;

		void init_geometry();
		void draw_stage();

		void init_player();
		void draw_player();

		void draw_rope();

		void init_shaders();
		void load_shader(GLuint shader, char *filename);

		GLuint vshader, fshader, shaderprogram, viewport_uniform, color_uniform;
		GLuint geometry_array, geometry_object;
		GLuint player_array, player_object;

		int geometry_vertices;

};

#endif //renderer_h
