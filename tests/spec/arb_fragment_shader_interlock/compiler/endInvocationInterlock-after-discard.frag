// [config]
// expect_result: pass
// glsl_version: 4.20
// require_extensions: GL_ARB_fragment_shader_interlock
// [end config]

/* The GL_ARB_fragment_shader_interlock spec says:
 *
 *    The beginInvocationInterlockARB() and endInvocationInterlockARB() may
 *    only be placed inside the function main() of a fragment shader and may
 *    not be called within any flow control.  These functions may not be
 *    called after a return statement in the function main(), but may be
 *    called after a discard statement.
 */
#version 420
#extension GL_ARB_fragment_shader_interlock: require

in float f;

void main()
{
	beginInvocationInterlockARB();

	if (f < 0.5)
		discard;

	endInvocationInterlockARB();
}
