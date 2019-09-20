// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_EXT_demote_to_helper_invocation
// [end config]

#version 140
#extension GL_EXT_demote_to_helper_invocation: require

void main()
{
    bool a = helperInvocationEXT();
}