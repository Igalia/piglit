/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: ${extensions}
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

uniform ${prefix}sampler${param.sampler} s;
attribute vec4 pos;
attribute int lod;
flat varying ${param.coord} size;

void main()
{
  gl_Position = pos;
  size = textureSize${param.sampler}(s
% if param.sampler != '2DRect' and param.sampler != 'Buffer':
    , lod
% endif
  );
}
