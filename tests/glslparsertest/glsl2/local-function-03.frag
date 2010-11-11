// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS - local function declarations are not allowed in GLSL 1.10. */
#version 110
void main() {
    float foo(float x);
    gl_FragColor = vec4(0.0, foo(0.5), 0.0, 1.0);
}

float foo(float x) {
    return x + 0.5;
}

