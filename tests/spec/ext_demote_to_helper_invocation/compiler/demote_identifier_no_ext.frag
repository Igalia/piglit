// [config]
// expect_result: pass
// glsl_version: 1.40
// [end config]

#version 140

void main()
{
    // When GL_EXT_demote_to_helper_invocation is disabled, `demote` can still
    // be used as an identifier.
    int demote = 1;
}