uniform int n;
void main()
{
	for (int i = 0; i < n; i++) {
		gl_TexCoord[i] = vec4(0.5, 0.5, 0.5, 0.5) * float(i);
	}
}
