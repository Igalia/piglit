/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: ${extensions}
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

uniform ${prefix}sampler${param.sampler} s;
attribute vec4 pos;
attribute ${param.coord} coord;
attribute ${param.grad} dPdx;
attribute ${param.grad} dPdy;
flat varying ${prefix}vec4 color;

void main()
{
  gl_Position = pos;
  color = ${param.func}Grad${offset}(s, coord, dPdx, dPdy
% if offset == 'Offset':
    , ${param.offsetcoord}(1)
% endif
  );
}
