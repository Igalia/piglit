// [config]
// expect_result: fail
// glsl_version: 1.50
// [end config]

#version 150

struct C {
    ivec4 fs_color;
};

in C c;
out vec4 color;

void main() {
  color = vec4(c.fs_color);
}
