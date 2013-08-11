env = Environment()
env['CCFLAGS'] = '-g -Wall -pedantic'

env.Program('main', Split('main.cc renderer.cc world.cc stage.cc geometry.cc player.cc hook.cc'),
	LIBS=Split('GL glut GLEW m'))
# GL seems to give -lm anyway
