#version 120

varying vec4 var0;
varying vec4 var1;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	var0 = vec4(0, 1, 2, 3);
	var1 = vec4(4, 5, 6, 7);
}

