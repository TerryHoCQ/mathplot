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
        //sm::vec<T, N> data;

        ssbo() {}
        ~ssbo() {}

        // Init is not built into the constructor, as client code must ensure there is an OpenGL context available
        void init()
        {
            glGenBuffers (1, &this->name);
            this->init_buffer_object();
        }

        void init_buffer_object()
        {
            sm::vec<T, N> data = {};
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            glBufferData (GL_SHADER_STORAGE_BUFFER, N * sizeof(T), data.data(), GL_STATIC_DRAW);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
        }

        // Copy data in the sm::vec data over to the GPU, once it has been initialized with init_buffer_object
        // data may contain less than N elements, and may be copied at an offset
        template<std::size_t _N = N>
        void copy_to_gpu (sm::vec<T, _N>& data, std::size_t offset = 0u)
        {
            static_assert (_N <= N);
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, offset * sizeof(T), _N * sizeof(T), GL_MAP_WRITE_BIT));
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            // Note: cpuptr is a 'pointer to the beginning of the mapped range' and so we don't incorporate offset again, below:
            for (unsigned int i = 0; i < _N; ++i) { cpuptr[i] = data[i]; }
            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
        }

        // Map the GPU memory to CPU space, then copy the values into this->data. NB: it's a
        // performance hit to *copy* to the mapped data to our sm::vec, because the data is
        // *already in CPU accessible memory* after glMapBufferRange().
        // However, in case you need it, here it is.
        template<std::size_t _N = N>
        void copy_from_gpu (sm::vec<T, _N>& data, std::size_t offset = 0u)
        {
            static_assert (_N <= N);
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, offset * sizeof(T), _N * sizeof(T), GL_MAP_READ_BIT));
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            for (unsigned int i = 0; i < _N; ++i) { data[i] = cpuptr[i]; }
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
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, N * sizeof(T), GL_MAP_READ_BIT));
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            for (unsigned int i = 0; i < N; ++i) { r.update (cpuptr[i]); }
            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            return r;
        }
    };

} // mplot::gl
