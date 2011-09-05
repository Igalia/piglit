#version 110

uniform sampler2DRectShadow tex;
varying vec4 texcoords;

void main() {
	gl_FragColor = shadow2DRect(tex, texcoords.xyy * vec3(31.0, 31.0, 1.0));
}
