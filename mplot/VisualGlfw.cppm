/*!
 * \file
 *
 * Singleton to manage init/deinit of GLFW3
 *
 * \author Seb James
 * \date March 2025
 */
module;

#ifndef _glfw3_h_
# define GLFW_INCLUDE_NONE
# include <GLFW/glfw3.h>
#endif

#include <iostream>
#include <cstdint>
#include <stdexcept>

export module mplot.visualglfw;

import mplot.gl.version;

export namespace mplot
{
    //! Singleton resource class for mplot::Visual scenes.
    template<std::int32_t glver>
    class VisualGlfw
    {
    private:
        VisualGlfw() { }
        ~VisualGlfw() { glfwTerminate(); }

        bool initialized = false;

    public:
        void init()
        {
            if (this->initialized) { return; } // as already initialized
            if (!glfwInit()) { std::cerr << "GLFW initialization failed!\n"; }

            // Set up error callback
            glfwSetErrorCallback (mplot::VisualGlfw<glver>::errorCallback);

            // The rest of this function may be right to call with each window?
            if constexpr (mplot::gl::version::gles (glver) == true) {
                glfwWindowHint (GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint (GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
            }
            glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, mplot::gl::version::major (glver));
            glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, mplot::gl::version::minor (glver));
#ifdef __APPLE__
            glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
            // Tell glfw that we'd like to do anti-aliasing.
            glfwWindowHint (GLFW_SAMPLES, 4);

            this->initialized = true;
        }

        //! This is connected to VisualResources::setContext to keep glfw out of VisualResources,
        //! whilst allowing VisualResources to own the window handle (as mplot::win_t*)
        static void setContext (GLFWwindow* window) { glfwMakeContextCurrent (window); }
        //! Connected to VisualResources::releaseContext
        static void releaseContext() { glfwMakeContextCurrent (nullptr); }

        //! An error callback function for the GLFW windowing library
        static void errorCallback (std::int32_t error, const char* description)
        {
            std::cerr << "Error: " << description << " (code "  << error << ")\n";
        }

        VisualGlfw(const VisualGlfw<glver>&) = delete;
        VisualGlfw& operator=(const VisualGlfw<glver> &) = delete;
        VisualGlfw(VisualGlfw<glver> &&) = delete;
        VisualGlfw & operator=(VisualGlfw<glver> &&) = delete;

        //! C++11 magic statics (N2660) instance public function.
        static auto& i()
        {
            static VisualGlfw instance;
            return instance;
        }
    };

} // namespace mplot
