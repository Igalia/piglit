/* FAIL: According to GLSL 1.20 page 20,
 * "The length method cannot be called on an array that has not been
 *  explicitly sized."
 */
#version 120
void main()
{
   float a[];
   a[4] = 4.0;
   float length = a.length();
}
