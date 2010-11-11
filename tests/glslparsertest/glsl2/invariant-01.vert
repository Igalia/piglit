// [config]
// expect_result: pass
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS - based on examples in the GLSL 1.20 spec */
#version 120

invariant gl_Position;

varying vec3 Color1;
invariant Color1;

invariant varying vec3 Color2;

void main() { }
