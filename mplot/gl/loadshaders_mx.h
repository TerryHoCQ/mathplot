/*
 * Multiple context-aware GL LoadShaders
 */

#pragma once

#include <mplot/gl/shaders.h>

#include <mplot/tools.h>
#include <cstdint>
#include <vector>
#include <iostream>
#include <cstring>
#include <memory>

namespace mplot::gl
{
    //! Shader loading code.
    uint32_t LoadShadersMX (const std::vector<mplot::gl::ShaderInfo>& shader_info, GladGLContext* glfn)
    {
        if (shader_info.empty()) { return 0; }

        uint32_t program = glfn->CreateProgram();

#ifdef GL_SHADER_COMPILER
        uint8_t shaderCompilerPresent = GL_FALSE;
        glfn->GetBooleanv (GL_SHADER_COMPILER, &shaderCompilerPresent);
        if (shaderCompilerPresent == GL_FALSE) {
            std::cerr << "Shader compiler NOT present!\n";
        } else {
            if constexpr (debug_shaders == true) {
                std::cout << "Shader compiler present\n";
            }
        }
#endif
        for (auto entry : shader_info) {
            uint32_t shader = glfn->CreateShader (entry.type);
            entry.shader = shader;
            // Test entry.filename. If this GLSL file can be read, then do so, otherwise,
            // compile the default version specified in the ShaderInfo
            std::unique_ptr<char[]> source;
            if constexpr (debug_shaders == true) {
                std::cout << "Check file exists for " << entry.filename << std::endl;
            }
            if (mplot::tools::fileExists (entry.filename)) {
                std::cout << "Using " << mplot::gl::shader_type_str(entry.type)
                          << " shader from the file " << entry.filename << std::endl;
                source = mplot::gl::ReadShader (entry.filename);
            } else {
                if constexpr (debug_shaders == true) {
                    std::cout << "Using compiled-in " << mplot::gl::shader_type_str(entry.type) << " shader\n";
                }
                source = mplot::gl::ReadDefaultShader (entry.compiledIn);
            }
            if (source == nullptr) {
                for (auto entry : shader_info) {
                    glfn->DeleteShader (entry.shader);
                    entry.shader = 0;
                }
                return 0;

            } else {
                if constexpr (debug_shaders == true) {
                    std::cout << "Compiling this shader: \n" << "-----\n";
                    std::cout << source.get() << "-----\n";
                }
            }
            int32_t slen = (int32_t)std::strlen (source.get());
            const char* sptr = source.get();
            glfn->ShaderSource (shader, 1, &sptr, &slen);
            glfn->CompileShader (shader);
            int32_t shaderCompileSuccess = GL_FALSE;
            char infoLog[512];
            glfn->GetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileSuccess);
            if (!shaderCompileSuccess) {
                glfn->GetShaderInfoLog(shader, 512, NULL, infoLog);
                std::cerr << "\nShader compilation failed!";
                std::cerr << "\n--------------------------\n\n";
                std::cerr << source.get();
                std::cerr << "\n\n--------------------------\n";
                std::cerr << infoLog << std::endl;
                std::cerr << "Exiting.\n";
                exit (2);
            }

            // Test glGetError:
            GLenum shaderError = glfn->GetError();
            if (shaderError == GL_INVALID_VALUE) {
                std::cerr << "Shader compilation resulted in GL_INVALID_VALUE\n";
                exit (3);
            } else if (shaderError == GL_INVALID_OPERATION) {
                std::cerr << "Shader compilation resulted in GL_INVALID_OPERATION\n";
                exit (4);
            } // shaderError is 0

            if constexpr (debug_shaders == true) {
                std::cout << "Successfully compiled a " << mplot::gl::shader_type_str(entry.type) << " shader!\n";
            }
            glfn->AttachShader (program, shader);
            glfn->DeleteShader (shader); // Note it's correct to glDeleteShader after attaching it to program
        }

        int32_t linked = 0;
        glfn->LinkProgram (program);
        glfn->GetProgramiv (program, GL_LINK_STATUS, &linked);
        if (!linked) {
            int32_t len = 0;
            glfn->GetProgramiv (program, GL_INFO_LOG_LENGTH, &len);
            {
                std::unique_ptr<char[]> log = std::make_unique<char[]>(len+1);
                glfn->GetProgramInfoLog (program, len, &len, log.get());
                std::cerr << "Shader linking failed: " << log.get() << std::endl << "Exiting.\n";
            }
            glfn->DeleteProgram (program);
            exit (5);
        } // else successfully linked

        return program;
    }
} // namespace
