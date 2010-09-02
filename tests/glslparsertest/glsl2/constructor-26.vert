/* PASS */
#version 120
struct s { float f; };

void main()
{
    s t = s(1); // an implicit conversion should happen here
}
