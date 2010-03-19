void main()
{
	gl_FragColor = mix(vec4(1.0, 0.0, 2.0, 0.0),
			   vec4(0.0, 1.0, 0.0, 2.0),
			   vec4(0.5, 0.5, 0.75, 0.25));

}
