// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL
 * 
 * GLSL 1.20 section 4.2 states:
 * "When a function name is redeclared in a nested scope, it hides all 
 *  functions declared with that name in the outer scope."
 *
 * Thus, declaring a new function exp(float, float) hides the builtin function.
 */
float exp(float x, float y)
{
    return x + y;
}

void main()
{
    float f = exp(2.0);
    gl_Position = vec4(0.0);
}
