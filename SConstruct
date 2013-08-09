env = Environment()
env['CCFLAGS'] = '-g -Wall'

env.Program('main', Split('main.cc renderer.cc world.cc stage.cc geometry.cc player.cc hook.cc'),
	LIBS=Split('GL glut GLEW'))

