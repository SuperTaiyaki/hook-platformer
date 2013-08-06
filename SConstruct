env = Environment()
env['CCFLAGS'] = '-g -Wall'

env.Program('main', Split('main.cc renderer.cc world.cc stage.cc'),
	LIBS=Split('GL glut GLEW'))

