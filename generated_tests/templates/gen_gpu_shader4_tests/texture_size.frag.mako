/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: ${extensions}
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

uniform ${prefix}sampler${param.sampler} s;
flat varying int lod;

void main()
{
  ${param.coord} v = textureSize${param.sampler}(s
% if param.sampler != '2DRect' and param.sampler != 'Buffer':
    , lod
% endif
  );

  gl_FragColor = vec4(v${swizzle});
}
