// [config]
// expect_result: pass
// glsl_version: 1.20
// [end config]

#version 120

int
f() {
    int x = 1 / 0;
    return x;
}

