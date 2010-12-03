/* From page 35 (page 41 of the PDF) of the GLSL 1.20 spec:
 *
 *    "The equality operators and assignment operator are only allowed if the
 *    two operands are same size and type. Structure types must be of the same
 *    declared structure. Both array operands must be explicitly sized."
 *
 * [config]
 * expect_result: fail
 * glsl_version: 1.20
 * [end config]
 */
#version 120
void main()
{
	int[] a = int[](0,1,2,3);
	int[] b;

	// These assignments will implicitly size b.
	b[0] = 0;
	b[1] = 1;
	b[2] = 2;
	b[3] = 3;

	gl_FrontColor = float(b == a) * vec4(0, 1, 0, 1);
}
