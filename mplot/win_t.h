module;

#ifndef _glfw3_h_ // glfw3 has not yet been externally included
# define GLFW_INCLUDE_NONE // Here, we tell GLFW that we will explicitly include GL3/gl3.h and GL/glext.h
# include <GLFW/glfw3.h>
#endif // _glfw3_h_

export module mplot.core:win_t;

export namespace mplot
{
    // With mplot::Visual, we use a GLFW window which is owned by mplot::Visual.
    using win_t = GLFWwindow;
}
