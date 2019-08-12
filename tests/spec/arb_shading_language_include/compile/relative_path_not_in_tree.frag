// [config]
// expect_result: fail
// glsl_version: 1.10
// require_extensions: GL_ARB_shading_language_include
// dummy_shader_include: True
// shader_include_path: /dummy/path_to
// [end config]

#extension GL_ARB_shading_language_include: enable

// This path does not exist
#include <shader_not_include>

void main()
{
}
