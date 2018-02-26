#version 120

varying vec4 var0;
varying vec4 var1;

void main()
{
	gl_FragColor = vec4(gl_PointCoord.xy * 1.1 - 0.05, 0, 0);

	if (var0 != vec4(0, 1, 2, 3) || var1 != vec4(4, 5, 6, 7))
		gl_FragColor.z = 1; /* something is wrong */
}
