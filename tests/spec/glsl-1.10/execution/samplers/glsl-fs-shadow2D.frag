#version 110

uniform sampler2DShadow tex;
varying vec4 texcoords;

void main() {
	gl_FragColor = shadow2D(tex, texcoords.xyy);
}
