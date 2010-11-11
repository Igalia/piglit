// [config]
// expect_result: pass
// glsl_version: 110
// [end config]

#if 1 == 0 || defined UNDEFINED
#else
#endif

void main()
{
  gl_Position = gl_Vertex;
}

