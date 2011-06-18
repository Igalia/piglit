varying vec4 texcoord;

void main()
{
	gl_Position = gl_Vertex;
	texcoord = (gl_Vertex + 1.0) / 2.0;
}
