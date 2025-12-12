#pragma once

/*
 * Common code for SSBO interactions in mplot programs
 *
 * Note: You have to include a header like gl3.h or glext.h etc for the GL types and
 * functions BEFORE including this file.
 *
 * Author: Seb James.
 */

#include <cstddef>
#include <sm/vec>
#include <sm/vvec>
#include <sm/range>
#include <mplot/gl/util_mx.h>

namespace mplot::gl
{
    // An SSBO and its data
    // @tparam index: The index of the buffer, used in the GLSL
    // @tparam T: The type of the data in the SSBO
    // @tparam N: The number of elements of type T in the SSBO
    // Could add version template params if necessary, to select correct gl function calls
    template <unsigned int index, typename T, std::size_t N> // T should be simple type?
    struct ssbo
    {
        // The name of the buffer, generated with glGenBuffers()
        unsigned int name = 0;
        // The CPU-side data for the buffer
        sm::vec<T, N> data;
        // The OpenGL function pointer
        GladGLContext* glfn = nullptr;

        ssbo() {}
        ~ssbo() {}

        // Init is not built into the constructor, as client code must ensure there is an OpenGL context available
        void init (GladGLContext* _glfn)
        {
            this->glfn = _glfn;
            this->glfn->GenBuffers (1, &this->name);
            this->copy_to_gpu();
        }

        // Copy the data in the sm::vec data over to the GPU
        void copy_to_gpu()
        {
            this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            this->glfn->BufferData (GL_SHADER_STORAGE_BUFFER, N * sizeof(T), this->data.data(), GL_STATIC_DRAW);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
        }

        // Map the GPU memory to CPU space, then copy the values into this->data. NB: it's a
        // performance hit to *copy* to the mapped data to our sm::vec, because the data is
        // *already in CPU accessible memory* after glMapBufferRange().
        // However, in case you need it, here it is.
        void copy_from_gpu()
        {
            this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            T* cpuptr = static_cast<T*>(this->glfn->MapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, N*sizeof(T), GL_MAP_READ_BIT));
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            for (unsigned int i = 0; i < N; ++i) { this->data[i] = cpuptr[i]; }
            this->glfn->UnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
        }

        // Find the range of the data in the given Shader Storage Buffer Object
        //
        // ssbo_idx: The Index of the Shader Storage Buffer Object that we're reading from
        // ssbo_name: The name (really a number) of the Shader Storage Buffer Object that we're reading from
        // ssbo_num_elements: The number of elements of type T in the SSBO.
        sm::range<T> get_range()
        {
            sm::range<T> r;
            r.search_init();
            this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            T* cpuptr = static_cast<T*>(this->glfn->MapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, N*sizeof(T), GL_MAP_READ_BIT));
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            for (unsigned int i = 0; i < N; ++i) { r.update (cpuptr[i]); }
            this->glfn->UnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            return r;
        }
    };

} // mplot::gl
