// [config]
// expect_result: fail
// glsl_version: 4.20
// require_extensions: GL_ARB_fragment_shader_interlock
// check_link: true
// [end config]

/* The GL_ARB_fragment_shader_interlock spec says:
 *
 *    A compile- or link-time error will be generated if main() calls either
 *    function more than once, contains a call to one function without a
 *    matching call to the other, or calls endInvocationInterlockARB() before
 *    calling beginInvocationInterlockARB().
 */
#version 420
#extension GL_ARB_fragment_shader_interlock: require

void main()
{
	endInvocationInterlockARB();
	beginInvocationInterlockARB();
}
