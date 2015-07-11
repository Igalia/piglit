/*[config]
 *   expect_result: fail
 *   glsl_version: 1.30
 * [end config]
 */

#version 130

void f(void)
{}

void main()
{
    ~f();
}
