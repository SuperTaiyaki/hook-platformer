#version 120

//out vec4 out_Color;
uniform vec3 Color;

void main(void) {
	gl_FragColor = vec4(Color.xyz, 1.0);
}

