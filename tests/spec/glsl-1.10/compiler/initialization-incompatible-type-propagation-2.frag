// [config]
// expect_result: fail
// glsl_version: 1.10
// [end config]
//
// Initializing a variable using the variable with a wrong type
// should not affect the type of the variable being initialized.
// While we cannot check emitted error message the test at least
// should not crash, see bug:
// https://bugs.freedesktop.org/show_bug.cgi?id=107547

#version 110

uniform struct {
    float field;
} data;

void f() {
    vec2 a = data;
    a.x -= 1.0;
}
