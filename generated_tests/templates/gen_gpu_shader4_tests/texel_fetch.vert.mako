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
attribute int lod;
flat varying ${prefix}vec4 color;

void main()
{
  gl_Position = pos;
  color = texelFetch${param.sampler}${offset}(s, coord
% if param.sampler != '2DRect' and param.sampler != 'Buffer':
    , lod
% endif
% if offset == 'Offset':
    , ${param.offsetcoord}(1)
% endif
  );
}
