// [config]
// expect_result: fail
// glsl_version: 1.10
// [end config]
//
void f(int i) {}

void main()
{
    f(1 += 2 % 3);
}
