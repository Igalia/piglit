/* PASS */
#version 120

void main()
{
  struct s { int i; };

  s temp[2] = s[2](s(1), s(2));
}
