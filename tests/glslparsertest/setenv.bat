@echo off

REM setenv.bat

REM This batch file sets up environment variables needed to build glslparsertest.
REM Please change this to reflect your local tree.
REM By default, it assumes that the 3rd party libraries are PEERs to the glslparsertest root.

SET PEER=%CD%\..

SET GLEW=%PEER%\glew
SET WXWIDGETS_LIB=%PEER%\wxMSW-2.4.2-3Dlabs-lib
SET WXWIDGETS_INC=%PEER%\wxMSW-2.4.2-3Dlabs-inc
