void main()
{
	gl_FrontColor = vec4(sqrt(0.0) + 0.75);
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

