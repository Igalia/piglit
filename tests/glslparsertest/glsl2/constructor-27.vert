/* FAIL - too many parameters given to structure constructor */
struct s { float f; };

void main()
{
    s t = s(1.0, 2.0);
}
