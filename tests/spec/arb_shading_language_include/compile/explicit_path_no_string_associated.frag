// [config]
// expect_result: fail
// glsl_version: 1.10
// require_extensions: GL_ARB_shading_language_include
// dummy_shader_include: True
// [end config]

#extension GL_ARB_shading_language_include: enable

// We enabled the dummy shader include so this path
// exists, but there is not string associated with
// "/dummy". There is only one associated with the
// full path "/dummy/path_to/shader_include"
#include </dummy>

void main()
{
}
