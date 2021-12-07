#ifndef shader_H
#define shader_H

#define GLEW_STATIC
#include <GL/glew.h>

// This is the content of the .h file, which is where the declarations go
GLuint initShader(const GLchar* vertexPath, const GLchar* fragmentPath);

					   // This is the end of the header guard
#endif