#pragma once

/*
 * Code for shader-related GL functionality in mathplot programs.
 *
 * Note: You have to include a header like gl3.h or glext.h etc for the GL types and
 * functions BEFORE including this file.
 *
 * Author: Seb James.
 */

#if defined __gl3_h_ || defined __gl_h_
// GL headers have been externally included
#else
# include <mplot/glad/gl.h>
#endif

#include <cstdint>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <memory>
#include <filesystem>

namespace mplot::gl
{
    /*!
     * Data structure for shader info.
     *
     * LoadShaders() takes an array of ShaderFile structures, each of which contains the
     * type of the shader, and a pointer a C-style character string (i.e., a
     * NULL-terminated array of characters) containing the filename of a GLSL file to
     * use, and another pointer to the compiled-in version of the shader.
     *
     * The array of structures is terminated by a final Shader with
     * the "type" field set to GL_NONE.
     *
     * LoadShaders() returns the shader program value (as returned by
     * glCreateProgram()) on success, or zero on failure.
     */
    struct ShaderInfo
    {
        // GLenum is, in practice, a 32 bit unsigned int. The type appears not to be defined in
        // OpenGL 3.1 ES (though it does appear in 3.2 ES), so here I use std::uint32_t.
        std::uint32_t type;
        std::string filename;
        std::string compiledIn;
        std::uint32_t shader;
    };

    // To enable debugging, set true.
    const bool debug_shaders = false;

    //! Read a shader from a file.
    std::unique_ptr<char[]> ReadShader (const std::string& filename)
    {
        if (!std::filesystem::is_regular_file (filename)) { // restrict to regular files
            std::cerr << "'" << filename << "' is not a regular file\n";
            return nullptr;
        }
        std::size_t len = std::filesystem::file_size (filename);
        std::unique_ptr<char[]> source = std::make_unique<char[]>(len + 1);
        std::ifstream fin (filename.c_str(), std::ios::in);
        if (!fin.is_open()) {
            std::cerr << "Unable to open file '" << filename << "'\n";
            return nullptr;
        }
        if (!fin.eof()) { fin.read (source.get(), len); }
        source[len] = 0;
        return source;
    }

    /*!
     * Read a default shader, stored as a const char*. ReadDefaultShader reads a
     * file: allocates some memory, copies the text into the new memory and then
     * returns a char* pointer to the memory.
     */
    std::unique_ptr<char[]> ReadDefaultShader (const std::string& shadercontent)
    {
        std::size_t len = shadercontent.size();
        std::unique_ptr<char[]> source = std::make_unique<char[]>(len + 1);
        std::memcpy (static_cast<void*>(source.get()), static_cast<const void*>(shadercontent.c_str()), len);
        source[len] = 0;
        return source;
    }

    std::string shader_type_str (uint32_t shader_type)
    {
        std::string type("unknown");
        if (shader_type == GL_VERTEX_SHADER) {
            type = "vertex";
        } else if (shader_type == GL_FRAGMENT_SHADER) {
            type = "fragment";
#ifdef GL_COMPUTE_SHADER
        } else if (shader_type == GL_COMPUTE_SHADER) {
            type = "compute";
#endif
        }
        return type;
    }

} // namespace
