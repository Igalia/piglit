#version 110

uniform sampler2DShadow tex;
varying vec4 tex_coord;

void main() {
	gl_FragColor = shadow2D(tex, tex_coord.xyy);
}
