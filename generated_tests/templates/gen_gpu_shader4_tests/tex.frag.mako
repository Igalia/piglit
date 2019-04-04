/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: ${extensions}
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

uniform ${prefix}sampler${param.sampler} s;
flat varying ${param.coord} coord;

void main()
{
  ${prefix}vec4 v = ${param.func}${offset}(s, coord
% if offset == 'Offset':
    , ${param.offsetcoord}(1)
% endif
  );

  gl_FragColor = vec4(v);
}
