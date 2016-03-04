// [config]
// expect_result: pass
// glsl_version: 1.50
// [end config]

#version 150

struct C {
    vec4 fs_color;
};

flat in C c;
out vec4 color;

void main() {
  color = vec4(c.fs_color);
}
