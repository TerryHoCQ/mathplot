/*
 * Common code for GL functionality in mathplot programs that use multicontext GLAD headers.
 *
 * Author: Seb James.
 */
module;

#if defined __gl3_h_ || defined __gl_h_
// GL headers have been externally included
#else
# include <mplot/glad/gl.h>
#endif

#include <cstdint>
#include <stdexcept>
#include <string>
#include <iostream>

export module mplot.gl.util;

export namespace mplot::gl::Util
{
    std::uint32_t checkError (const char *file, std::int32_t line, GladGLContext* glfn)
    {
        std::uint32_t errorCode = 0;
        std::uint32_t ecount = 0;
        std::string error;

        while ((errorCode = glfn->GetError()) != GL_NO_ERROR) {

            switch (errorCode) {
            case GL_INVALID_ENUM:
            {
                error = "GL error: GL_INVALID_ENUM";
                break;
            }
            case GL_INVALID_VALUE:
            {
                error = "GL error: GL_INVALID_VALUE";
                break;
            }
            case GL_INVALID_OPERATION:
            {
                error = "GL error: GL_INVALID_OPERATION";
                break;
            }
            case 1283: // Not part of GL3?
            {
                error = "GL error: GL_STACK_OVERFLOW";
                break;
            }
            case 1284: // Not part of GL3?
            {
                error = "GL error: GL_STACK_UNDERFLOW";
                break;
            }
            case GL_OUT_OF_MEMORY:
            {
                error = "GL error: GL_OUT_OF_MEMORY";
                break;
            }
            case GL_INVALID_FRAMEBUFFER_OPERATION:
            {
                error = "GL error: GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            }
            default:
            {
                error = "GL checkError: Unknown GL error code";
                break;
            }
            }
            std::cout << error << " | " << file << ":" << line << std::endl;
            ++ecount;
        }
        if (ecount) { throw std::runtime_error (error); }

        return errorCode;
    }
} // namespace
