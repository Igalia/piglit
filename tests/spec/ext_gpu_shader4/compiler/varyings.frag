/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * require_extensions: GL_EXT_gpu_shader4
 * [end config]
 */
#extension GL_EXT_gpu_shader4 : require

noperspective varying float temperature;
flat varying vec3 myColor;
centroid varying vec2 myTexCoord1;
noperspective varying vec2 myTexCoord2;
centroid noperspective varying vec2 myTexCoord3;
noperspective centroid varying vec2 myTexCoord4;

void main()
{
    gl_FragColor = vec4(1.0);
}
