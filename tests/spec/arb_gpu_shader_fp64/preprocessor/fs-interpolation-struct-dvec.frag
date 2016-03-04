// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_gpu_shader_fp64
// [end config]

#version 150
#extension GL_ARB_gpu_shader_fp64 : require

struct C {
    dvec4 fs_color;
};

in C c;
out vec4 color;

void main() {
  color = vec4(c.fs_color);
}
