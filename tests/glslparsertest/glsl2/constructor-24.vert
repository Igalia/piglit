#version 120
uniform mat3 m3;
varying mat2 m2;
void main()
{
   m2 = mat2(m3);
}
