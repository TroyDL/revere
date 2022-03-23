
#include "gl_utils.h"
#include "r_utils/r_exception.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GL_CLAMP_TO_EDGE 0x812F 

using namespace std;
using namespace r_utils;

// Simple helper function to load an image into a OpenGL texture with common settings
void load_texture_from_file(const string& filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename.c_str(), &image_width, &image_height, NULL, 4);
    if(!image_data)
        R_THROW(("Unable to load image file: %s", filename.c_str()));

    auto image_texture = load_texture_from_rgba(image_data, image_width * image_height * 4, image_width, image_height);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;
}

GLuint load_texture_from_rgba(const uint8_t* pixels, size_t size, uint16_t w, uint16_t h)
{
    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    return image_texture;
}