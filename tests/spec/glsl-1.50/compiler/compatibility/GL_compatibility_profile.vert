// [config]
// expect_result: pass
// glsl_version: 1.50compatibility
// [end config]
#version 150 compatibility

#ifndef GL_compatibility_profile
    #error GL_compatibility_profile undefined
#else
    #if GL_compatibility_profile != 1
	#error GL_compatibility_profile has incorrect value
    #endif
#endif

void func() {}
