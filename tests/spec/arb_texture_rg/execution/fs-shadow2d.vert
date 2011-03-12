#version 110

varying vec4 tex_coord;

void main() {
    gl_Position = gl_Vertex;
	tex_coord = (gl_Vertex + 1.0) / 2.0;
}
