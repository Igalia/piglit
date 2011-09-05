#version 110
#extension GL_EXT_texture_array : enable

uniform sampler1DArrayShadow tex;
varying vec4 texcoords;

void main() {
	gl_FragColor = shadow1DArray(tex, vec3(texcoords.x, 0.0, texcoords.y));
}
