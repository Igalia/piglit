// [config]
// expect_result: fail
// glsl_version: 1.10
// require_extensions: GL_ARB_shading_language_include
// dummy_shader_include: True
// shader_include_path: /dummy
// [end config]

#extension GL_ARB_shading_language_include: enable

// This path exists but there will be no string
// is associated with it.
#include <path_to>

void main()
{
}
