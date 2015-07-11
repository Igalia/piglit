/*[config]
 *   expect_result: fail
 *   glsl_version: 1.10
 * [end config]
 */

#version 110

void f(void)
{}

void main()
{
    float y = f();
}
