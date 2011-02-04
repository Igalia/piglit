// [config]
// expect_result: pass
// glsl_version: 1.30
// [end config]

#version 130

int
f() {
    int x = 1 % 0;
    return x;
}

