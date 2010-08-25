/* PASS */

struct str {
  float params[4];
};

void main()
{
  str s;

  for (int i = 0; i < 4; ++i)
    s.params[i] = 1.0;

  gl_FragColor = vec4(str.params[0], str.params[1],
		      str.params[2], str.params[3]);
}
