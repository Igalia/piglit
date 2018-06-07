
Piglit
======

1. About
2. Setup
3. How to run tests
4. Available test sets
5. How to write tests
6. Todo


1. About
--------

Piglit is a collection of automated tests for OpenGL and OpenCL
implementations.

The goal of Piglit is to help improve the quality of open source
OpenGL and OpenCL drivers by providing developers with a simple means to
perform regression tests.

The original tests have been taken from
- Glean ( http://glean.sf.net/ ) and
- Mesa ( http://www.mesa3d.org/ )


2. Setup
--------

First of all, you need to make sure that the following are installed:

  - Python 2.7.x
  - Python Mako module
  - numpy (http://www.numpy.org)
  - six (https://pypi.python.org/pypi/six)
  - cmake (http://www.cmake.org)
  - GL, glu and glut libraries and development packages (i.e. headers)
  - X11 libraries and development packages (i.e. headers)
  - waffle (http://www.waffle-gl.org)
  - mako

Optionally, you can install the following:

  - lxml. An accelerated python xml library using libxml2 (http://lxml.de/)
  - simplejson. A fast C based implementation of the python json library.
    (https://simplejson.readthedocs.org/en/latest/)
  - jsonstreams. A JSON stream writer for python.
    (https://jsonstreams.readthedocs.io/en/stable/)

For Python 2.x you can install the following to add features, these are
unnecessary for python3:
  - backports.lzma. A backport of python3's lzma module to python2,
    this enables fast native xz (de)compression in piglit for results files
    (https://github.com/peterjc/backports.lzma)
  - subprocess32. A backport of the subprocess from python3.2, which includes
    timeout support. This only works for Linux

For testing the python framework using "py.test unittests/framework"
  - py.test. A python test framework, used for running the python framework
    test suite.
  - tox. A tool for testing python packages against multiple configurations of
    python (https://tox.readthedocs.org/en/latest/index.html)
  - mock. A python module for mocking other python modules. Required only for
    unittests (https://github.com/testing-cabal/mock)
  - psutil. A portable process library for python
  - jsonschema. A JSON validator library for python
  - pytest-mock. A mock plugin for pytest
  - pytest-pythonpath. A plugin for pytest to do automagic with sys.path
  - pytest-raises. A plugin for pytest that allows decorating tests that expect
    failure
  - pytest-warnings. A plugin for pytest that handles python warnings
  - pytest-timeout. A plugin for pytest to timeout tests.

Now configure the build system:

  $ ccmake .

This will start cmake's configuration tool, just follow the onscreen
instructions. The default settings should be fine, but I recommend you:
 - Press 'c' once (this will also check for dependencies) and then
 - Set "CMAKE_BUILD_TYPE" to "Debug"
Now you can press 'c' again and then 'g' to generate the build system.
Now build everything:

  $ make


### 2.1 Cross Compiling

On Linux, if cross-compiling a 32-bit build on a 64-bit host, first make sure
you didn't have CMakeCache.txt file left from 64-bit build (it would retain old
flags), then you must invoke cmake with options "-DCMAKE_SYSTEM_PROCESSOR=x86
-DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32".


### 2.2 Ubuntu

Install development packages.
  $ sudo apt-get install cmake g++ mesa-common-dev libgl1-mesa-dev python-numpy python-mako freeglut3-dev x11proto-gl-dev libxrender-dev libwaffle-dev

Configure and build.
  $ cmake .
  $ make


### 2.3 Mac OS X

Install CMake.
http://cmake.org/cmake/resources/software.html
Download and install 'Mac OSX Universal' platform.

Install Xcode.
http://developer.apple.com/xcode

Configure and build.
  $ cmake .
  $ make


### 2.4 Cygwin

Install development packages.
  - cmake
  - gcc4
  - make
  - opengl
  - libGL-devel
  - python
  - python-numpy
  - libglut-devel

Configure and build.
  $ cmake .
  $ make


### 2.5 Windows

Install Python 3.
http://www.python.org/download

Install CMake.
http://cmake.org/cmake/resources/software.html
Download and install 'Windows' platform.

Download and install Ninja
https://github.com/ninja-build/ninja/releases

Install MinGW-w64
https://mingw-w64.org/

Download OpenGL Core API and Extension Header Files.
http://www.opengl.org/registry/#headers
Pass -DGLEXT_INCLUDE_DIR=/path/to/headers

Install python mako.
> pip install mako

Install NumPy.
> pip install numpy


#### 2.5.1 GLUT

Download freeglut for Mingw.
http://www.transmissionzero.co.uk/software/freeglut-devel/

> cmake -H. -Bbuild -G "Ninja" -DGLEXT_INCLUDE_DIR=\path\to\glext -DGLUT_INCLUDE_DIR=\path\to\freeglut\include -DGLUT_glut_LIBRARY=\path\to\freeglut\lib\x64\libfreeglut.a -DGLEXT_INCLUDE_DIR=\path\to\glext
> ninja -C build


#### 2.5.2 Waffle

Download and build waffle for MinGW.
http://www.waffle-gl.org/

Open the Command Prompt.
CD to piglit directory.

> cmake -H. -Bbuild -G "Ninja" -DGLEXT_INCLUDE_DIR=\path\to\glext -DPIGLIT_USE_WAFFLE=TRUE -DWAFFLE_INCLUDE_DIRS=\path\to\waffle\include\waffle WAFFLE_LDFLAGS=\path\to\waffle\lib\libwaffle-1.a


3. How to run tests
-------------------

Make sure that everything is set up correctly:

  $ ./piglit run sanity results/sanity

You may include '.py' on the profile, or you may exclude it (sanity vs sanity.py),
both are equally valid.

You may also preface test profiles with tests/ (or any other path you like),
which may be useful for shell tab completion.

You may provide multiple profiles to be run at the same time:

  $ ./piglit run quick_cl gpu deqp_gles3 results/gl-cl-combined

Use

  $ ./piglit run
  or
  $ ./piglit run -h

To learn more about the command's syntax.

Have a look into the tests/ directory to see what test profiles are available:

  $ ls tests/*.py

See also section 4.

To create some nice formatted test summaries, run

  $ ./piglit summary html summary/sanity results/sanity

Hint: You can combine multiple test results into a single summary.
      During development, you can use this to watch for regressions:

  $ ./piglit summary html summary/compare results/baseline results/current

      You can combine as many testruns as you want this way (in theory;
      the HTML layout becomes awkward when the number of testruns increases)

Have a look at the results with a browser:

  $ xdg-open summary/sanity/index.html

The summary shows the 'status' of a test:

 pass   This test has completed successfully.

 warn   The test completed successfully, but something unexpected happened.
        Look at the details for more information.

 fail   The test failed.

 crash  The test binary exited with a non-zero exit code.

 skip   The test was skipped.

 timeout  The test ran longer than its allotted time and was forcibly killed.


There are also dmesg-* statuses. These have the same meaning as above, but are
triggered by dmesg related messages.


### 3.1 Environment Variables

There are a number of environment variables that control the way piglit
behaves.

 PIGLIT_COMPRESSION
      Overrides the compression method used. The same values that piglit.conf
      allows for core:compression.

 PIGLIT_PLATFORM
      Overrides the platform run on. These allow the same values as ``piglit
      run -p``. This values is honored by the tests themselves, and can be used
      when running a single test.

 PIGLIT_FORCE_GLSLPARSER_DESKTOP
      Force glslparser tests to be run with the desktop (non-gles) version of
      glslparsertest. This can be used to test ES<x>_COMPATABILITY extensions
      for OpenGL

 PIGLIT_NO_FAST_SKIP
       Piglit has a mechanism run in the python layer for skipping tests with
       unmet OpenGL or window system dependencies without starting a new
       process (which is expensive). Sometimes this system doesn't work or is
       undesirable, setting this environment variable to True will disable this
       system.

 PIGLIT_NO_TIMEOUT
       When this variable is true in python then any timeouts given by tests
       will be ignored, and they will run until completion or they are killed.


### 3.2 Note

The way 'piglit run' and 'piglit summary' count tests are different, 'piglit
run' counts the number of Test derived instance in the profile(s) selected,
while 'piglit summary' counts the number of subtests a result contains, or it's
result if there are no subtests. This means that the number shown by 'piglit
run' will be less than or equal to the number calculated by 'piglit summary'.


### 3.3 Shell Completions

Piglit has completions for bash, located in completions/bash/piglit. Once this
file is sourced into bash `piglit` and `./piglit` will have tab completion
available. For global availability place the file somewhere that bash will
source the file on startup. If piglit is installed and bash-completions are
available, then this completion file will be installed system-wide.


4. Available test sets
----------------------

Test sets are specified as Python scripts in the tests directory.
The following test sets are currently available:


### 4.1 OpenGL Tests

sanity.py
    This suite contains minimal OpenGL sanity tests. These tests must
    pass, otherwise the other tests will not generate reliable results.

all.py
    This suite contains all OpenGL tests.

quick.py
    Run all tests, but cut down significantly on their runtime
    (and thus on the number of problems they can find).

gpu.py
	A further reduced set of tests from quick.py, this runs tests only
	for hardware functionality and not tests for the software stack.

llvmpipe.py
	A reduced set of tests from gpu.py removing tests that are problematic
	using llvmpipe

cpu.py
	This profile runs tests that don't touch the gpu, in other words all of
	the tests in quick.py that are not run by gpu.py

glslparser.py
	A subset of all.py which runs only glslparser tests

shader.py
	A subset of all.py which runs only shader tests

no_error.py
	A modified version of the test list run as khr_no_error variants


### 4.2 OpenCL Tests

cl.py
    This suite contains all OpenCL tests.

quick_cl.py
	This runs all of the tests from cl.py as well as tests from opencv
	and oclconform.


### 4.3 External Integration

xts.py
	Support for running the X Test Suite using piglit.

igt.py
	Support for running Intel-gpu-tools test suite using piglit.

deqp_egl.py
	Support for running dEQP's EGL profile with piglit.

deqp_gles2.py
	Support for running dEQP's gles2 profile with piglit.

deqp_gles3.py
	Support for running dEQP's gles3 profile with piglit.

deqp_gles31.py
	Support for running dEQP's gles3.1 profile with piglit.

deqp_vk.py
	Support for running the official Khronos Vulkan CTS profile with piglit.

khr_gl.py
	Support for running the open source Khronos OpenGL CTS tests with piglit.

khr_gl45.py
	Support for running the open source Khronos OpenGL 4.5 CTS tests with piglit.

cts_gl.py
	Support for running the closed source Khronos OpenGL CTS tests with piglit.

cts_gl45.py
	Support for running the closed source Khronos OpenGL 4.5 CTS tests with piglit.

cts_gles.py
	Support for running the closed source Khronos GLES CTS tests with piglit.

oglconform.py
	Support for running sub-test of the Intel oglconform test suite with piglit.


5. How to write tests
---------------------

Every test is run as a separate process. This minimizes the impact that
severe bugs like memory corruption have on the testing process.

Therefore, tests can be implemented in an arbitrary standalone language.
C is the preferred language for compiled tests, piglit also supports its own
simple formats for test shaders and glsl parser input.

All new tests must be added to the appropriate profile, all.py profile for
OpenGL and cl.py for OpenCL. There are a few basic test classes supported by the
python framework:

 PiglitBaseTest
   A shared base class for all native piglit tests.

   It starts each test as a subprocess, captures stdout and stderr, and waits
   for the test to return.

   It provides test timeouts by setting the instances 'timeout' attribute to an
   integer > 0 which is the number of seconds the test should run.

   It interprets output by reading stdout and looking for 'PIGLIT: ' in the
   output, and then reading any trailing characters as well formed json
   returning the test result.

   This is a base class and should not be used directly, but provides an
   explanation of the behavior of the following classes.

 PiglitGLTest
   A test class for native piglit OpenGL tests.

   In addition to the properties of PiglitBaseTest it provides a mechanism for
   detecting test window resizes and rerunning tests as well as keyword
   arguments for platform requirements.

 PiglitCLTest
   A test class for native piglit OpenCL tests.

   It currently provides no special features.

 GLSLParserTest
   A class for testing a glsl parser.

   It is generally unnecessary to call this class directly as it uses a helper
   function to search directories for tests.

 ShaderTest
   A class for testing using OpenGL shaders.

   It is generally unnecessary to call this class directly as it uses a helper
   function to search directories for tests.


6. Integration
--------------

Piglit provides integration for other test suites as well. The rational for
this is that it provides piglit's one process per test protections (one test
crashing does not crash the whole suite), and access to piglit's reporting
tools.

Most integration is done through the use of piglit.conf, or through environment
variables, with piglit.conf being the preferred method.


### 6.1 dEQP

Piglit provides a generic layer for dEQP based test suites, and specific
integration for several suites.

I suggest using Chad Versace's repo of dEQP, which contains a gbm target.
https://github.com/chadversary/deqp

It should be built as follows:
cmake . -DDEQP_TARGET=gbm -GNinja

Additional targets are available in the targets directory. gbm isn't compatible
for most (any?) blob driver, so another target might be necessary if that is a
requirement. One of the x11_* targets or drm is probably a good choice.

The use of ninja is optional.

Once dEQP is built add the following information to piglit.conf, which can
either be located in the root of the piglit repo, or in $XDG_CONFIG_HOME
(usually $HOME/.config).

"""
[deqp-gles2]
bin=<deqp source dir>/modules/gles2/deqp-gles2

[deqp-gles3]
bin=<deqp source dir>/modules/gles3/deqp-gles3

[deqp-gles31]
bin=<deqp source dir>/modules/gles31/deqp-gles31
"""

These platforms can be run using deqp_gles*.py as a suite in piglit.
For example: ./piglit run deqp_gles31 my_results -c

It is also possible to mix integrated suites and piglit profiles together:
./piglit run deqp_gles31 quick cl my_results

dEQP profiles generally contain all of the tests from the previous profile, so
gles31 covers gles3 and gles2.


### 6.2 Khronos CTS

Add the following to your piglit.conf file:

"""
[cts]
bin=<cts source dir>/cts/glcts
"""
