// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_EXT_demote_to_helper_invocation
// [end config]

#version 140
#extension GL_EXT_demote_to_helper_invocation: require

void main()
{
    // When the extension is enabled, using `demote` as an identifier must fail.
    int demote = 1;
}