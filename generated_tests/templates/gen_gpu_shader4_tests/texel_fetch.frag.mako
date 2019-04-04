/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: ${extensions}
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

uniform ${prefix}sampler${param.sampler} s;
flat varying ${param.coord} coord, offset;
flat varying int lod;

void main()
{
  ${prefix}vec4 v = texelFetch${param.sampler}${offset}(s, coord
% if param.sampler != '2DRect' and param.sampler != 'Buffer':
    , lod
% endif
% if offset == 'Offset':
    , ${param.offsetcoord}(1)
% endif
  );

  gl_FragColor = vec4(v);
}
