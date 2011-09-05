#version 110
#extension GL_EXT_texture_array : enable

uniform sampler2DArrayShadow tex;
varying vec4 texcoords;

void main() {
	gl_FragColor = shadow2DArray(tex, vec4(texcoords.x, texcoords.y, 0.0, texcoords.y));
}
