// [config]
// expect_result: pass
// glsl_version: 4.40
// check_link: false
// [end config]

#version 440

/* At one point, Mesa broke inout parameter qualifier handling when
 * trying to validate the xfb_buffer enhanced layouts qualifier.
 */
void
f(inout float x)
{
    x += 1.0;
}

