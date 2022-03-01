/* Start Header -------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the 
prior written consent of DigiPen
Institute of Technology is prohibited.
Language: C++
Platform: Windows
Author: Jon Sanchez
End Header --------------------------------------------------------*/
#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

// In order to use it, Debug output and the following setup function 
// have to be provided to OpenGL:
//              glEnable(GL_DEBUG_OUTPUT);
//              glDebugMessageCallback(MessageCallback, 0);
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                const GLchar * message, const void * userParam);
