// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

void main()
{
    mat2 m1,m2;
    bool b = m1 > m2;  // greater-than operator can operate on matrices, however, equal (==) and not equal (!=) operators can be used with matrices
}
