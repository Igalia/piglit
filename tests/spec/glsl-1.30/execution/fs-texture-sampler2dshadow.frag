#version 130
const float canary = 0.125;
uniform sampler2DShadow tex;
in vec4 tex_coord;
void main() {
	float s = texture(tex, tex_coord.xyy);
	gl_FragColor = vec4(s, canary, canary, canary);
}
