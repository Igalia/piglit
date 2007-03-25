// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999,2000  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT




// geomrend.h:  convenience object for rendering any geometry via
// a host of OpenGL paths: immediate mode (glVertex), vertex
// arrays with glDrawArrays, vertex arrays with glArrayElement,
// vertex arrays with glDrawElements, and any of the preceding
// methods stuffed in a display list.

#ifndef __geomrend_h__
#define __geomrend_h__

#include "glwrap.h"
#include <cassert>

namespace GLEAN {

// A helper class to store parameter array data.
class ArrayData {
public:
    GLint size;
    GLenum type;
    GLsizei stride;
    const GLvoid* pointer;
    
    ArrayData();
    void setData(GLint sizeIn, GLenum typeIn, GLsizei strideIn, const GLvoid* pointerIn);
};

class GeomRenderer {
 public:
    // These indicate the methods of passing the primitive data to OpenGL.  Note that whether
    // the arrays are locked or not is an independent variable, not part of the method.  See
    // setArraysLocked and getArraysLocked.
    enum DrawMethod {GLVERTEX_MODE, GLARRAYELEMENT_MODE, GLDRAWARRAYS_MODE, GLDRAWELEMENTS_MODE};

    // Sorry, no indices, and especially no silly edge flags. There's no vertex bit because
    // vertex data always implicitly enabled (you can't draw anything without vertex data).
    enum ParameterBits {COLOR_BIT = 1, TEXTURE_COORD_BIT = 2, NORMAL_BIT = 4};

    // Only a default constructor.
    GeomRenderer();
    
    // Used to set the method by which this GeomRenderer will pass the primitive data to the GL.
    // Default is GLVERTEX_MODE.
    void setDrawMethod(DrawMethod);
    DrawMethod getDrawMethod() const;
    
    // Used to set the various parameters that are either enabled or disabled.  Example usage:
    // to tell the GeomRenderer to pass vertex, color, and texcoord data, but not normals,
    // call setParameterBits(COLOR_BIT | TEXTURE_COORD_BIT).  (Vertex data is implicitly enabled
    // all the time.)  The default is that only vertex data is enabled.
    void setParameterBits(GLuint bits);
    GLuint getParameterBits() const;
    
    // Used to specify whether EXT_compiled_vertex_array should be used if present.  Default is false.
    // If set to true, the arrays are kept unlocked and only locked just before rendering calls are issued.
    // If you call setArraysCompiled(true) and the extension is not present, the function returns false
    // and acts as though you had passed false in as the argument.
    bool setArraysCompiled(bool);
    bool getArraysCompiled() const;

    // If you're using GLDRAWELEMENTS_MODE, GLARRAYELEMENT_MODE, or GLVERTEX_MODE, you need to give
    // it the indices to pass into the GL.
    void setVArrayIndices(GLuint count, GLenum type, const GLvoid* indices);

    // This hands the actual primitive data to the GeomRenderer.  It holds onto these as pointers,
    // rather than copying them, so don't delete the data until you're done with the GeomRenderer.
    // These are prototypically equivalent to their respective GL calls, except that there's an extra
    // argument on the front of the vertex function for how many elements are in the array (this is
    // atomic; if you pass in 5, it means there are 5 vertices, not 5 floats or bytes or whatever).
    // The lengths of all other arrays are assumed to be >= the size passed in for the vertex array.
    void setVertexPointer(GLuint arrayLength, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void setColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void setTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
    void setNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer);

    // Finally, the actual calls to do something with all this data.  You can either choose to render
    // it given the configuration, or generate a display list of rendering it with the given
    // configuration (uses GL_COMPILE mode to build the list).  Fails if insufficient data has
    // been given (i.e. if you don't give it an array for an enabled parameter, if you don't
    // give it an array of indices when it needs them).
    bool renderPrimitives(GLenum mode);
    bool generateDisplayList(GLenum mode, GLint& listHandleOut);

 private:
    bool isReadyToRender();

    // Helper functions for unpacking and translating the data from the indices, vertices, colors,
    // texcoords, and normals arrays.
    GLuint getIndex(int);
    void sendVertex(GLuint index);
    void sendColor(GLuint index);
    void sendTexCoord(GLuint index);
    void sendNormal(GLuint index);

    DrawMethod drawMethod;
    GLuint parameterBits;
    bool compileArrays;
    
    GLuint indicesCount;
    GLenum indicesType;
    const GLvoid* indices;

    GLuint arrayLength;
    
    ArrayData vertexData;
    ArrayData colorData;
    ArrayData texCoordData;
    ArrayData normalData;
};
   
} // namespace GLEAN

#endif // __geomrend_h__



