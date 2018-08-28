// [config]
// expect_result: pass
// glsl_version: 4.20
// require_extensions: GL_ARB_fragment_shader_interlock
// check_link: true
// [end config]

#version 420
#extension GL_ARB_fragment_shader_interlock: require

void main()
{
	beginInvocationInterlockARB();
	endInvocationInterlockARB();
}
