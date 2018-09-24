/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 */
struct Foo
{
  float does_exist_member;
};

void
main(void)
{
  Foo foo;
  foo.does_not_exist_member /= 3.0;
  gl_Position = vec4(1.0);
}
