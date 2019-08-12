// [config]
// expect_result: fail
// glsl_version: 1.10
// require_extensions: GL_ARB_shading_language_include
// [end config]

#extension GL_ARB_shading_language_include: enable

// We did not enable the dummy shader include so
// this path does not exist.
#include </dummy/path_to/shader_include>

void main()
{
}
