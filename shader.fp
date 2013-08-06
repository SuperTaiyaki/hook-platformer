// Fallback version
#version 120

//out vec4 out_Color;

void main(void) {
	//out_Color = vec4(1.0, 1.0, 1.0, 1.0);
/*	if (gl_FragCoord.x > 600) {
		discard;
	}*/
	gl_FragColor = vec4(1.0, 1.0, 0.5, 1.0);
}

