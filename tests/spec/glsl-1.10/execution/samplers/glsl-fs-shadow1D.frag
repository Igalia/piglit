#version 110

uniform sampler1DShadow tex;
varying vec4 texcoords;

void main() {
	gl_FragColor = shadow1D(tex, vec3(texcoords.x, 0.0, texcoords.y));
}
