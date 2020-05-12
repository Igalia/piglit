/* [config]
 * expect_result: pass
 * glsl_version: 4.30
 * [end config]
 */
 
#version 430

const vec3[] array1c = {vec3(0, 1, 1), vec3(1,1, 0), };
const vec3[] array2c = {{-5, -8, 3}, {0, 6, 6}, };
const vec3[] array3c = {{2, 4, 8, }, {1, 3, 4, }};
const vec3[] array4c = vec3[](vec3(1, 5, 4), vec3(2, 6, 5));
const vec3[] array5c = vec3[](array1c[0], array1c[1]);
const vec3[] array6c = {array5c[0], array5c[1]};
const vec3[] array7c = {array5c[1], array5c[0], };

vec3[] array1 = {array6c[0], array6c[1]};
vec3[] array2 = {array7c[0], array7c[1], };
vec3[] array3 = vec3[](array5c[0], array5c[1]);
vec3[] array4 = {vec3(1, 2, 3), vec3(4, 5, 6)};
vec3[] array5 = {vec3(3, 2, 1), vec3(6, 5, 4), };
vec3[] array6 = {{9, 8, 7}, {6, 5, 4}, };
vec3[] array7 = {{6, 5, 4}, {3, 2, 1}};

out vec4 fragColor;

void main ()
{
	fragColor = vec4(0, 1, 0, 1);
}
