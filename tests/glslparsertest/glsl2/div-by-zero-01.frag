// [config]
// expect_result: pass
// glsl_version: 1.20
// [end config]

#version 120

float
f() {
    float x = 1.0 / 0.0;
    return x;
}

