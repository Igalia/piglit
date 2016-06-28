// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// test that the precise qualifier must appear first with multiple qualifications,
// without ARB_shading_language_420pack.

// the spec says the order of qualification must be:
//	precise-qualifier invariant-qualifier interpolation-qualifier ...

#version 130
#extension GL_MESA_shader_integer_functions: require

invariant precise vec4 x;
