// [config]
// expect_result: fail
// glsl_version: 1.20
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

#version 120
#extension GL_ARB_shader_storage_buffer_object: require

/* From the GL_ARB_shader_storage_buffer_object spec:
 *
 *  "Such unsized arrays may be indexed with general integer expressions, but
 *   may not be passed as an argument to a function or indexed with a negative
 *   constant expression."
 */

buffer a {
	int s[];
};

int foo(int array[2]) {
	return array[0];
}

void main(void)
{
	s[1] = int(0);
	int val = foo(s);
}
