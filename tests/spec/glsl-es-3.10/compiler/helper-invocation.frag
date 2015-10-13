// [config]
// expect_result: pass
// glsl_version: 3.10 es
// [end config]
//

#version 310 es
out highp vec4 color;
void main()
{
        if (gl_HelperInvocation)
                discard;
        color = vec4(0.0);
}
