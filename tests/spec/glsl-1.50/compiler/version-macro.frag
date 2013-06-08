// [config]
// expect_result: pass
// glsl_version: 1.50
// [end config]

#version 150
int x[int(__VERSION__ == 150)];
