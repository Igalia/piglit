// [config]
// expect_result: pass
// glsl_version: 1.30
// [end config]

#version 130

uint
f() {
    uint x = 1u / 0u;
    return x;
}

