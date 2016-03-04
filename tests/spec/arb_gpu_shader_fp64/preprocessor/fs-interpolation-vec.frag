// [config]
// expect_result: pass
// glsl_version: 1.50
// [end config]

#version 150

in vec4 fs_color;
out vec4 color;

void main() {
  color = vec4(fs_color);
}
