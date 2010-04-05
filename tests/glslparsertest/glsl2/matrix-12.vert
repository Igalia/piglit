/* PASS */

uniform mat4 a;

void main()
{
    gl_Position = vec4(a[1][0]);
}
