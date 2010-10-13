/* FAIL - bitwise operations aren't supported in 1.20. */
#version 120
void main()
{
    int x = ~0 - 1;
}
