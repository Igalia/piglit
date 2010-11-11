// [config]
// expect_result: fail
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - variable name conflicts with function name in same scope
 * 
 * See also redeclaration-09.vert (passes in 1.10) and redeclaration-11.vert
 * (where the variable declaration is legal, but still hides the function).
 */
#version 120
float foo()
{
    return 0.5;
}

const float foo = 1.0;
