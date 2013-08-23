// [config]
// expect_result: pass
// glsl_version: 3.30
// [end config]

#version 330
int x[int(__VERSION__ == 330)];
