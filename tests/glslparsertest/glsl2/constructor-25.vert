/* FAIL - implicit conversions are not allowed in GLSL 1.10 */
struct s { float f; };

void main()
{
    s t = s(1);
}
