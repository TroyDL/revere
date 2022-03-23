
#ifndef __revere_gl_utils_h
#define __revere_gl_utils_h

#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <string>

void load_texture_from_file(const std::string& filename, GLuint* out_texture, int* out_width, int* out_height);

#endif
