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
attribute float lod;
flat varying ${prefix}vec4 color;

void main()
{
  gl_Position = pos;
  color = ${param.func}Lod${offset}(s, coord, lod
% if offset == 'Offset':
    , ${param.offsetcoord}(1)
% endif
  );
}
