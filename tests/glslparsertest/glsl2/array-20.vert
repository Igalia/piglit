/* FAIL - arrays passed as parameters must declare a size */
void func(in vec4 vertices[])
{
    gl_Position = gl_Vertex;
}
