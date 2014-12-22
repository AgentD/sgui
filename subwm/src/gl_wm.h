/*
 * gl_wm.h
 * This file is part of sgui
 *
 * Copyright (C) 2012 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef GL_WM_H
#define GL_WM_H



#include "sgui_subwm_predef.h"
#include "sgui_ctx_window.h"
#include "sgui_internal.h"
#include "sgui_ctx_wm.h"
#include "sgui_event.h"
#include "sgui_skin.h"



#ifndef SGUI_NO_OPENGL
#ifdef SGUI_WINDOWS
    #include <windows.h>
#endif

#ifdef SGUI_OS_X
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#ifndef GLAPIENTRY
    #define GLAPIENTRY APIENTRY
#endif

#ifndef GL_VERSION_2_0
    typedef char GLchar;
#endif

#ifndef GL_FRAGMENT_SHADER
    #define GL_FRAGMENT_SHADER 0x8B30
#endif

#ifndef GL_VERTEX_SHADER
    #define GL_VERTEX_SHADER 0x8B31
#endif

typedef GLuint (GLAPIENTRY * GLCREATESHADERPROC) (GLenum type);
typedef void (GLAPIENTRY * GLLINKPROGRAMPROC) (GLuint program);
typedef GLuint (GLAPIENTRY * GLCREATEPROGRAMPROC) (void);
typedef void (GLAPIENTRY * GLCOMPILESHADERPROC) (GLuint shader);
typedef void (GLAPIENTRY * GLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void (GLAPIENTRY * GLSHADERSOURCEPROC) (GLuint shader,
                                                GLsizei count,
                                                const GLchar** string,
                                                const GLint* length);
typedef void (GLAPIENTRY * GLATTACHSHADERPROC) (GLuint program,
                                                GLuint shader);
typedef void (GLAPIENTRY * GLGENVERTEXARRAYSPROC) (GLsizei n,
                                                   GLuint* arrays);
typedef void (GLAPIENTRY * GLGETPROGRAMINFOLOGPROC) (GLuint program,
                                                     GLsizei bufSize,
                                                     GLsizei* length,
                                                     GLchar* infoLog);
typedef void (GLAPIENTRY * GLGETSHADERINFOLOGPROC) (GLuint shader,
                                                    GLsizei bufSize,
                                                    GLsizei* length,
                                                    GLchar* infoLog);

typedef void (GLAPIENTRY * GLBINDFRAGDATALOCATIONPROC) (GLuint program,
                                                        GLuint colorNumber,
                                                        const GLchar* name);
typedef void (GLAPIENTRY * GLBINDATTRIBLOCATIONPROC) (GLuint program,
                                                      GLuint index,
                                                      const GLchar* name);

typedef GLint (GLAPIENTRY * GLGETUNIFORMLOCATIONPROC) (GLuint program,
                                                       const GLchar* name);
typedef void (GLAPIENTRY * GLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (GLAPIENTRY * GLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (GLAPIENTRY * GLUNIFORMMATRIX4FVPROC) (GLint location,
                                                    GLsizei count,
                                                    GLboolean transpose,
                                                    const GLfloat* value);




typedef struct
{
    GLCREATESHADERPROC CreateShader;
    GLLINKPROGRAMPROC LinkProgram;
    GLCREATEPROGRAMPROC CreateProgram;
    GLCOMPILESHADERPROC CompileShader;
    GLGENBUFFERSPROC GenBuffers;
    GLSHADERSOURCEPROC ShaderSource;
    GLATTACHSHADERPROC AttachShader;
    GLGENVERTEXARRAYSPROC GenVertexArrays;
    GLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
    GLGETSHADERINFOLOGPROC GetShaderInfoLog;
    GLGETUNIFORMLOCATIONPROC GetUniformLocation;
    GLUNIFORM1FPROC Uniform1f;
    GLUNIFORM1IPROC Uniform1i;
    GLUNIFORMMATRIX4FVPROC UniformMatrix4fv;
    GLBINDFRAGDATALOCATIONPROC BindFragDataLocation;
    GLBINDATTRIBLOCATIONPROC BindAttribLocation;
}
sgui_gl_functions;

typedef struct
{
    sgui_ctx_wm super;

    GLuint wndtex;
}
sgui_gl_wm;

typedef struct
{
    sgui_gl_wm super;

    GLint u_tex0;
    GLint u_tex1;
    GLint u_alpha;
    GLint u_mvp;

    GLuint vao;         /**< \brief VAO with window VBO and IBO bound */
    GLuint vbo;         /**< \brief VBO with window vertices */
    GLuint ibo;         /**< \brief IBO with window indices */

    GLuint vsh;         /**< \brief vertex shader for drawing windows */
    GLuint fsh;         /**< \brief fragment shader for drawing windows */
    GLuint prog;        /**< \brief shader program  for drawing windows */

    sgui_gl_functions gl;
}
sgui_gl_core_wm;



#ifdef __cplusplus
extern "C" {
#endif

sgui_ctx_wm* gl_wm_create( sgui_window* wnd );

sgui_ctx_wm* gl_wm_create_core( sgui_window* wnd );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_NO_OPENGL */

#endif /* GL_WM_H */

