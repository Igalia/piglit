/* PASS */
uniform sampler2DRectShadow s;
varying vec3 coord;

void main()
{
  gl_FragColor = shadow2DRect(s, coord);
}
