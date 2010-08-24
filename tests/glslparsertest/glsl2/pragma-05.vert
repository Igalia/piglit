/* FAIL
 *
 * From page 11 (page 17 of the PDF) of the GLSL 1.10 spec:
 *
 *     "[#pragma optimize] can only be used outside function definitions."
 */
void main()
{
#pragma optimize(off)
  gl_Position = gl_Vertex;
}
