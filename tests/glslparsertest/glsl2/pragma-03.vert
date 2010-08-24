/* FAIL
 *
 * From page 11 (page 17 of the PDF) of the GLSL 1.10 spec:
 *
 *     "[#pragma debug] can only be used outside function definitions."
 */
void main()
{
#pragma debug(off)
  gl_Position = gl_Vertex;
}
