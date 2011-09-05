#version 110

varying vec4 texcoords;

void main() {
	gl_Position = gl_Vertex;
	texcoords = (gl_Vertex + 1.0) / 2.0;
}
