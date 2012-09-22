Types of GL test frameworks
===========================

Class piglit_gl_framework is an abstract class whose interface is used to
drive GL tests. There are three main subclasses:

1. piglit_glut_framework
------------------------

If you configure Piglit to build without Waffle, then this is the only
framework that gets built. It uses GLUT to interact with the GL and the window
system. It does not support running tests in FBO mode; that is, for each test,
a window is created and shown on the screen.

2. piglit_winsys_framework
--------------------------

This framework works behaves similarly to piglit_glut_framework, except that
it uses Waffle to interact to interact with the GL and the window system.
This framework has a different backing implementation for each supported
window system.

If you configure Piglit to build with Waffle, each test will usually choose
this framework if it is ran without the -fbo argument.

3. piglit_fbo_framework
-----------------------

This framework, after creating a window, uses it only for making the GL
context current and attemtps to prevent it from appearing on the screen. For
rendering, it instead creates an FBO and sets it as the read and draw buffer.

If you configure Piglit to build with Waffle, each test will usually attempt
to use this framework if it is ran with the -fbo argument.
