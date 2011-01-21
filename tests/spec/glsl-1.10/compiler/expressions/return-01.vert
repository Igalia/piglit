// [config]
// expect_result: pass
// glsl_version: 1.10
// [end config]

#ifdef GL_ES
precision mediump float;
#endif

void A(int i) { }

void B(int i) {
	// This is crazy, but not technically illegal.  The value of A(i) has
	// void type, and the return type of the function is void.
	return A(i);
}

attribute vec4 vertex;

void main() {
	B(1);
	gl_Position = vertex;
}
