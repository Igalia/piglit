/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * [end config]
 *
 *
 *  From Section 6.3 (Iteration) the GLSL 4.60 spec:
 *     "Non-terminating loops are allowed. The consequences of very long or non-terminating
 *      loops are platform dependent."
 *
 */

uniform bool k;
void main()
{
   gl_FragColor = vec4(1.5);
   if(!k) return;
   do {
       gl_FragColor += vec4(1.5);
   } while(k);
}

