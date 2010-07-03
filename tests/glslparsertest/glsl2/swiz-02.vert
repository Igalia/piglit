/* FAIL: assignment of a vec2 to a float */
void main()
{
	float a;
	vec4 b;

	b.x = 6.0;
	a = b.xy;
}
