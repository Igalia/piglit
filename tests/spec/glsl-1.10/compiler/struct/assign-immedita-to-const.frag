/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * [end config]
 *
 * Test for bug in TPPStreamCompiler::assignOperands?
 */
struct S {
	float f;
};

void F(S s) {}

const S s = S(0.0);

void F()
{
	F(s);
}

void main()
{
	gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
}
