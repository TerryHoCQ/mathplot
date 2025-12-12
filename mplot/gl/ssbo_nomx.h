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
#include <mplot/gl/util_nomx.h>

namespace mplot::gl
{
    // An SSBO and its data
    // @tparam index: The index of the buffer, used in the GLSL
    // @tparam T: The type of the data in the SSBO
    // @tparam N: The number of elements of type T in the SSBO
    template <unsigned int index, typename T, std::size_t N> // Could add version template params if necessary, to select correct gl function calls
    struct ssbo
    {
        // The name of the buffer, generated with glGenBuffers()
        unsigned int name = 0;
        // The CPU-side data for the buffer
        sm::vec<T, N> data;

        ssbo() {}
        ~ssbo() {}

        // Init is not built into the constructor, as client code must ensure there is an OpenGL context available
        void init()
        {
            glGenBuffers (1, &this->name);
            this->copy_to_gpu();
        }

        // Copy the data in the sm::vec data over to the GPU
        void copy_to_gpu()
        {
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            glBufferData (GL_SHADER_STORAGE_BUFFER, N * sizeof(T), this->data.data(), GL_STATIC_DRAW);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
        }

        // Map the GPU memory to CPU space, then copy the values into this->data. NB: it's a
        // performance hit to *copy* to the mapped data to our sm::vec, because the data is
        // *already in CPU accessible memory* after glMapBufferRange().
        // However, in case you need it, here it is.
        void copy_from_gpu()
        {
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, N*sizeof(T), GL_MAP_READ_BIT));
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            //this->data.resize(N);
            for (unsigned int i = 0; i < N; ++i) { this->data[i] = cpuptr[i]; }
            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
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
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, N*sizeof(T), GL_MAP_READ_BIT));
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            for (unsigned int i = 0; i < N; ++i) { r.update (cpuptr[i]); }
            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            return r;
        }
    };

} // mplot::gl
