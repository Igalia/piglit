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

struct Data {
    float field;
};

void f() {
    Data a = vec2(0.0);
    a.field -= 1.0;
}
