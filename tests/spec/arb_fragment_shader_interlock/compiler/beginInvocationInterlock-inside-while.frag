// [config]
// expect_result: fail
// glsl_version: 4.20
// require_extensions: GL_ARB_fragment_shader_interlock
// check_link: false
// [end config]

/* The GL_ARB_fragment_shader_interlock spec says:
 *
 *    A compile- or link-time error will be generated if main() calls either
 *    function more than once, contains a call to one function without a
 *    matching call to the other, or calls endInvocationInterlockARB() before
 *    calling beginInvocationInterlockARB().
 *
 * and
 *
 *    (8) What restrictions should be imposed on the use of the
 *        beginInvocationInterlockARB() and endInvocationInterlockARB() functions
 *        delimiting a critical section?
 *
 *      RESOLVED:  We impose restrictions similar to those on the barrier()
 *      built-in function in tessellation control shaders to ensure that any
 *      shader using this functionality has a single critical section that can
 *      be easily identified during compilation...
 *
 * The GLSL 4.60 spec says:
 *
 *    For tessellation control shaders, the barrier() function may only be
 *    placed inside the function main() of the tessellation control shader and
 *    may not be called within any control flow. Barriers are also disallowed
 *    after a return statement in the function main(). Any such misplaced
 *    barriers result in a compile-time error.
 *
 * From this we infer that the first errors mentioned in the
 * GL_ARB_fragment_shader_interlock spec are intended to generate compile-time
 * errors.
 */
#version 420
#extension GL_ARB_fragment_shader_interlock: require

void main()
{
	while (true) {
		beginInvocationInterlockARB();
		break;
	}

	endInvocationInterlockARB();
}
