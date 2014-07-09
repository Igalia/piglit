/* -*- mode: c; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2; coding: utf-8-unix -*- */
/*
  Copyright (c) 2010 Kristóf Ralovich

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/


#include "piglit-util-gl.h"
#include "glsl-fs-raytrace-bug27060.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint program = -1;
static float rot[9] = {1,0,0,  0,1,0,   0,0,1};
static const float failing_pixel_percentage = 0.15F;

static const char* vsSource =
  "varying vec2 rayDir;                                                \n"
  "                                                                    \n"
  "void main()                                                         \n"
  "{                                                                   \n"
  "  rayDir = gl_MultiTexCoord0.xy - vec2(0.5,0.5);                    \n"
  "  gl_Position = gl_ProjectionMatrix * gl_Vertex;                    \n"
  "}\n";

static const char* fsSource =
  "const float INF     = 9999.9;                                       \n"
  "const float EPSILON = 0.00001;                                      \n"
  "const vec3 lightPos = vec3(0.0, 8.0, 1.0);                          \n"
  "const vec4 backgroundColor = vec4(0.2,0.3,0.4,1);                   \n"
  "                                                                    \n"
  "varying vec2 rayDir;                                                \n"
  "                                                                    \n"
  "uniform mat3 rot;                                                   \n"
  "                                                                    \n"
  "struct Ray                                                          \n"
  "{                                                                   \n"
  "vec3 orig;                                                          \n"
  "vec3 dir;                                                           \n"
  "};                                                                  \n"
  "                                                                    \n"
  "struct Sphere                                                       \n"
  "{                                                                   \n"
  "  vec3 c;                                                           \n"
  "  float r;                                                          \n"
  "};                                                                  \n"
  "                                                                    \n"
  "struct Isec                                                         \n"
  "{                                                                   \n"
  "  float t;                                                          \n"
  "  int idx;                                                          \n"
  "  vec3 hit;                                                         \n"
  "  vec3 n;                                                           \n"
  "};                                                                  \n"
  "                                                                    \n"
#ifdef __APPLE__
  "Sphere spheres0 = Sphere( vec3(0.0,0.0,-1.0), 0.5 );                \n"
  "Sphere spheres1 = Sphere( vec3(-3.0,0.0,-1.0), 1.5 );               \n"
  "Sphere spheres2 = Sphere( vec3(0.0,3.0,-1.0), 0.5 );                \n"
  "Sphere spheres3 = Sphere( vec3(2.0,0.0,-1.0), 1.0 );                \n"
#else
  "const Sphere spheres0 = Sphere( vec3(0.0,0.0,-1.0), 0.5 );          \n"
  "const Sphere spheres1 = Sphere( vec3(-3.0,0.0,-1.0), 1.5 );         \n"
  "const Sphere spheres2 = Sphere( vec3(0.0,3.0,-1.0), 0.5 );          \n"
  "const Sphere spheres3 = Sphere( vec3(2.0,0.0,-1.0), 1.0 );          \n"
#endif
  "                                                                    \n"
  "// Mesa intel gen4 generates \"unsupported IR in fragment shader 13\" for\n"
  "// sqrt, let's work around.                                         \n"
  "float                                                               \n"
  "sqrt_hack(float f2)                                                 \n"
  "{                                                                   \n"
  "  vec3 v = vec3(f2,0.0,0.0);                                        \n"
  "  return length(v);                                                 \n"
  "}                                                                   \n"
  "                                                                    \n"
  "void                                                                \n"
  "intersect(const in Ray ray,                                         \n"
  "          const in Sphere sph,                                      \n"
  "          const in int idx,                                         \n"
  "          inout Isec isec)                                          \n"
  "{                                                                   \n"
  "  // Project both o and the sphere to the plane perpendicular to d  \n"
  "  // and containing c. Let x be the point where the ray intersects  \n"
  "  // the plane. If |x-c| < r, the ray intersects the sphere.        \n"
  "  vec3 o = ray.orig;                                                \n"
  "  vec3 d = ray.dir;                                                 \n"
  "  vec3 n = -d;                                                      \n"
  "  vec3 c = sph.c;                                                   \n"
  "  float r = sph.r;                                                  \n"
  "  float t = dot(c-o,n)/dot(n,d);                                    \n"
  "  vec3 x = o+d*t;                                                   \n"
  "  float e = length(x-c);                                            \n"
  "  if(e > r)                                                         \n"
  "  {                                                                 \n"
  "    // no intersection                                              \n"
  "    return;                                                         \n"
  "  }                                                                 \n"
  "                                                                    \n"
  "  // Apply Pythagorean theorem on the (intersection,x,c) triangle   \n"
  "  // to get the distance between c and the intersection.            \n"
  "//#define BUGGY_INTEL_GEN4_GLSL                                       \n"
  "#ifndef BUGGY_INTEL_GEN4_GLSL                                       \n"
  "  float f = sqrt(r*r - e*e);                                        \n"
  "#else                                                               \n"
  "  float f = sqrt_hack(r*r - e*e);                                   \n"
  "#endif                                                              \n"
  "  float dist = t - f;                                               \n"
  "  if(dist < 0.0)                                                    \n"
  "  {                                                                 \n"
  "    // inside the sphere                                            \n"
  "    return;                                                         \n"
  "  }                                                                 \n"
  "                                                                    \n"
  "  if(dist < EPSILON)                                                \n"
  "    return;                                                         \n"
  "                                                                    \n"
  "  if(dist > isec.t)                                                 \n"
  "    return;                                                         \n"
  "                                                                    \n"
  "  isec.t = dist;                                                    \n"
  "  isec.idx = idx;                                                   \n"
  "                                                                    \n"
  "  isec.hit  = ray.orig + ray.dir * isec.t;                          \n"
  "  isec.n = (isec.hit - c) / r;                                      \n"
  "}                                                                   \n"
  "                                                                    \n"
  "Isec                                                                \n"
  "intersect(const in Ray ray,                                         \n"
  "          const in float max_t /*= INF*/)                           \n"
  "{                                                                   \n"
  "  Isec nearest;                                                     \n"
  "  nearest.t = max_t;                                                \n"
  "  nearest.idx = -1;                                                 \n"
  "                                                                    \n"
  "  intersect(ray, spheres0, 0, nearest);                             \n"
  "  intersect(ray, spheres1, 1, nearest);                             \n"
  "  intersect(ray, spheres2, 2, nearest);                             \n"
  "  intersect(ray, spheres3, 3, nearest);                             \n"
  "                                                                    \n"
  "  return nearest;                                                   \n"
  "}                                                                   \n"
  "                                                                    \n"
  "vec4                                                                \n"
  "idx2color(const in int idx)                                         \n"
  "{                                                                   \n"
  "  vec4 diff;                                                        \n"
  "  if(idx == 0)                                                      \n"
  "    diff = vec4(1.0, 0.0, 0.0, 0.0);                                \n"
  "  else if(idx == 1)                                                 \n"
  "    diff = vec4(0.0, 1.0, 0.0, 0.0);                                \n"
  "  else if(idx == 2)                                                 \n"
  "    diff = vec4(0.0, 0.0, 1.0, 0.0);                                \n"
  "  else if(idx == 3)                                                 \n"
  "    diff = vec4(1.0, 1.0, 0.0, 0.0);                                \n"
  "  return diff;                                                      \n"
  "}                                                                   \n"
  "                                                                    \n"
  "vec4                                                                \n"
  "trace0(const in Ray ray)                                            \n"
  "{                                                                   \n"
  "  Isec isec = intersect(ray, INF);                                  \n"
  "                                                                    \n"
  "  if(isec.idx == -1)                                                \n"
  "  {                                                                 \n"
  "    return backgroundColor;                                         \n"
  "  }                                                                 \n"
  "                                                                    \n"
  "  vec4 diff = idx2color(isec.idx);                                  \n"
  "                                                                    \n"
  "  vec3 N = isec.n;                                                  \n"
  "  vec3 L = normalize(lightPos-isec.hit);                            \n"
  "  vec3 camera_dir = normalize(ray.orig - isec.hit);                 \n"
  "  return dot(N,L)*diff + pow(                                       \n"
  "    clamp(dot(reflect(-L,N),camera_dir),0.0,1.0),16.0);             \n"
  "}                                                                   \n"
  "                                                                    \n"
  "vec4                                                                \n"
  "trace1(const in Ray ray)                                            \n"
  "{                                                                   \n"
  "  Isec isec = intersect(ray, INF);                                  \n"
  "                                                                    \n"
  "  if(isec.idx == -1)                                                \n"
  "  {                                                                 \n"
  "    return backgroundColor;                                         \n"
  "  }                                                                 \n"
  "                                                                    \n"
  "  Ray reflRay = Ray(isec.hit, reflect(ray.dir, isec.n));            \n"
  "                                                                    \n"
  "  vec4 reflCol = trace0(reflRay);                                   \n"
  "                                                                    \n"
  "  vec4 diff = idx2color(isec.idx) + reflCol;                        \n"
  "                                                                    \n"
  "  vec3 N = isec.n;                                                  \n"
  "  vec3 L = normalize(lightPos-isec.hit);                            \n"
  "  vec3 camera_dir = normalize(ray.orig - isec.hit);                 \n"
  "  return dot(N,L)*diff + pow(                                       \n"
  "    clamp(dot(reflect(-L,N),camera_dir),0.0,1.0),16.0);             \n"
  "}                                                                   \n"
  "                                                                    \n"
  "void main()                                                         \n"
  "{                                                                   \n"
  "  const float z = -0.5;                                             \n"
  "  const vec3 cameraPos = vec3(0,0,3);                               \n"
  "  Ray r = Ray(cameraPos, normalize(vec3(rayDir, z) * rot));         \n"
  "  gl_FragColor = trace1(r);                                         \n"
  "}\n";

enum piglit_result
piglit_display(void)
{
  int passed_cnt = 0;
  GLint location = glGetUniformLocation(program, "rot");
  static const float m = -10.F;
  static const float p =  10.F;
  static const float d = -0.5F;
  int x,y;

  glUseProgram(program);
  glUniformMatrix3fv(location, 1, 0, rot);

  glBegin(GL_QUADS);
  {
    glTexCoord2f(0.0F, 0.0F); glVertex3f(m, m, d);
    glTexCoord2f(1.0F, 0.0F); glVertex3f(p, m, d);
    glTexCoord2f(1.0F, 1.0F); glVertex3f(p, p, d);
    glTexCoord2f(0.0F, 1.0F); glVertex3f(m, p, d);
  }
  glEnd();
  glUseProgram(0);

  /* the pre-computed image is 256x256 */
  assert(piglit_height == 256);
  assert(piglit_width == 256);

  for (y = 0; y < piglit_height; y++)
  {
    for (x = 0; x < piglit_width; x++)
    {
      float color[3];

      color[0] = (float)gimp_image.pixel_data[(y*256+x)*3 +0] / 256.0F;
      color[1] = (float)gimp_image.pixel_data[(y*256+x)*3 +1] / 256.0F;
      color[2] = (float)gimp_image.pixel_data[(y*256+x)*3 +2] / 256.0F;

      if(piglit_probe_pixel_rgb(x, 255-y, color))
        passed_cnt++;
    }
  }

  piglit_present_results();

  return ((float)passed_cnt > (1.0F-failing_pixel_percentage)
          *piglit_width*piglit_height)
    ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
  GLint vs=-1, fs=-1;

  glDisable(GL_DEPTH_TEST);

  piglit_require_gl_version(20);

  glViewport(0, 0, piglit_width, piglit_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-10, 10, -10, 10, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vsSource);
  fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fsSource);

  assert(glIsShader(vs));
  assert(glIsShader(fs));

  program = piglit_link_simple_program(vs, fs);

  assert(glIsProgram(program));

  if (!piglit_link_check_status(program))
      piglit_report_result(PIGLIT_FAIL);
}
