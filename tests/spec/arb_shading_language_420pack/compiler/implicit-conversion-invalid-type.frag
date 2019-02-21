/* [config]
 * expect_result: fail
 * glsl_version: 1.30
 * require_extensions: GL_ARB_shading_language_420pack
 * [end config]
 */
#version 130
#extension GL_ARB_shading_language_420pack: enable

out vec4 color;

int test()
{
	/* Return invalid type, this should not succeed. */
	return ivec2(0);
}

void main()
{
        color = vec4(test());
}
