/* [config]
 * expect_result: fail
 * glsl_version: 3.10 es
 * require_extensions: GL_OES_shader_io_blocks
 * [end config]
 *
 * The GL_OES_shader_io_blocks spec says:
 *
 * "Issues
 *
 *      (1) What functionality was removed from interface blocks relative to
 *          GL 4.4?
 *
 *        - Interactions with features not supported by the underlying
 *          ES 3.1 API and Shading Language, including:
 *            * gl_ClipDistance shader inputs and outputs.
 *            * "component" layout
 *            * location aliasing
 *            * fragment shader output "index" layout
 *            * fragment shader gl_FragDepth layout "depth*" qualifiers
 *            * double-precision scalars and vectors
 *            * matching across shader stages with different qualifiers (other
 *              than precision and "in"/"out").
 *            * References allowing or assuming more than one shader object per
 *              pipeline stage.
 *            * gl_PerFragment is not added (only exists in compatibility
 *              profile)."
 */

#version 310 es
#extension GL_OES_shader_io_blocks: require
precision highp float;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_ClipDistance[2];
};

void main(void)
{
    gl_Position = vec4(0);
    gl_ClipDistance[0] = 0.5;
    gl_ClipDistance[1] = 0.5;
}
