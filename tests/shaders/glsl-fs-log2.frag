uniform vec4 args1, args2;

void main()
{
	gl_FragColor = log2(args1) + args2;
}
