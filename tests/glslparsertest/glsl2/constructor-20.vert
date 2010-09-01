/* PASS */
#version 120

void main()
{
  struct s { int i; };

  s temp[] = s[](s(1), s(2));
}
