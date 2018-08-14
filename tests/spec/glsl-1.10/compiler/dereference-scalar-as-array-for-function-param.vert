// [config]
// expect_result: fail
// glsl_version: 1.10
// [end config]

void func(int x)
{
    return;
}

void main()
{
	func(0[2]);
}
