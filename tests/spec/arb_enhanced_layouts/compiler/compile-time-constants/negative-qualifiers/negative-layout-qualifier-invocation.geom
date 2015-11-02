// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_gpu_shader5 GL_ARB_enhanced_layouts
// check_link: false
// [end config]
//
// Test to detect negative value of layout qualifier 'stream'
//
// From ARB_gpu_shader5 spec:
//
// "If an implementation supports <N> vertex streams, the
//     individual streams are numbered 0 through <N>-1"
//
// "If any stream declaration specifies a non-existent stream number,
//  the shader will fail to compile."

#version 150
#extension GL_ARB_gpu_shader5: enable
#extension GL_ARB_enhanced_layouts: require

const int start = 3;
layout(lines, invocations = start - 5) in;

void main()
{
}
