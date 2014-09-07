// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_tessellation_shader
// check_link: true
// [end config]

/**
 * From issue 42:
 *
 *   As a result, we choose a heavy-handed approach in which we only allow
 *   calls to barrier() inside main().  Even within main, barrier() calls are
 *   forbidden inside loops (even those that turn out to have constant loop
 *   counts and don't execute "break" or "continue" statements), if
 *   statements, or after a return statement.
 *
 * Further, from the spec text:
 *
 *   In particular, barrier() may not be called inside
 *   a switch statement, in either sub-statement of an if statement, inside a
 *   do, for, or while loop, or at any point after a return statement in the
 *   function main().
 *
 * Technically, we should disallow this usage of barrier() inside the always-taken
 * default case -- but this is an interesting edge case for Mesa's compiler, which
 * at this time does not express switch statements in the IR, and so an always-taken
 * default case is indistinguishable from code outside the switch.
 */

#version 150
#extension GL_ARB_tessellation_shader: require
layout(vertices = 3) out;
uniform int val;

void main() {
    gl_out[gl_InvocationID].gl_Position = vec4(0.0);
    switch (val) {
    default:
        barrier();
    }
    gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);
    gl_TessLevelInner = float[2](1.0, 1.0);
}
