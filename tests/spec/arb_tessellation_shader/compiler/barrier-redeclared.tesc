// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_tessellation_shader
// check_link: true
// [end config]

/**
 * ARB_tessellation_shader imposes tight restrictions on where calls to the
 * built-in function barrier() may appear.
 *
 * If the app provides its own function named 'barrier', then the usual suppression
 * of the builtin function should apply, and the app-provided function should be
 * freely callable in any context.
 */

#version 150
#extension GL_ARB_tessellation_shader: require
layout(vertices = 3) out;

void barrier() {}	/* suppresses the builtin */

void calls_barrier() {
    barrier();		/* in non-main OK */
}

void main() {
    gl_out[gl_InvocationID].gl_Position = vec4(0.0);

    for (int i = 0; i < 3; i++)
        barrier();	/* in loops OK */

    calls_barrier();

    gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);
    gl_TessLevelInner = float[2](1.0, 1.0);

    if (gl_in[0].gl_Position.x < 0) {
        barrier();	/* in control flow OK */
        return;
    }

    barrier();          /* after return OK */
}
