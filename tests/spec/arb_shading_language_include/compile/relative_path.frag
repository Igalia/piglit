// [config]
// expect_result: pass
// glsl_version: 1.10
// require_extensions: GL_ARB_shading_language_include
// dummy_shader_include: True
// shader_include_path: /dummy/path_to
// [end config]

#extension GL_ARB_shading_language_include: enable

#include <shader_include>

void main()
{
}
