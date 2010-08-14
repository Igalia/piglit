/* PASS */
uniform sampler2DRect s;
varying vec2 coord;

void main()
{
  gl_FragColor = texture2DRect(s, coord);
}
