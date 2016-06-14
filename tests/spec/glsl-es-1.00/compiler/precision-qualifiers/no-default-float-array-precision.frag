// [config]
// expect_result: fail
// glsl_version: 1.00
// check_link: true
// [end config]
//
// From section 4.5.3 ("Default Precision Qualifiers") of the GLSL ES
// 1.00 spec:
//
//     "The fragment language has no default precision qualifier for
//     floating point types. Hence for float, floating point vector
//     and matrix variable declarations, either the declaration must
//     include a precision qualifier or the default float precision
//     must have been previously declared."

#version 100

varying float f[4];

void main()
{
    gl_FragColor = vec4(f[0], f[1], f[2], f[3]);
}
