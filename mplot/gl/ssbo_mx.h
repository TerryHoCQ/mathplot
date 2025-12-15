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
    template <unsigned int index, typename T, std::size_t N> // requires std::is_arithmetic_v<T>?
    struct ssbo
    {
        // The name of the buffer, generated with glGenBuffers()
        unsigned int name = 0;
        // The OpenGL function pointer
        GladGLContext* glfn = nullptr;
        // Our CPU side data buffer
        sm::vec<T, N> data = {};

        bool ready() const { return this->name != 0u; }

        ssbo() {}
        ~ssbo() {}

        // Init is not built into the constructor, as client code must ensure there is an OpenGL context available
        void init (GladGLContext* _glfn)
        {
            this->glfn = _glfn;
            this->glfn->GenBuffers (1, &this->name); // has to happen after VBO buffers?
            this->init_buffer_object();
        }

        void init_buffer_object()
        {
            sm::vec<T, N> zd = {};
            this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            this->glfn->BufferData (GL_SHADER_STORAGE_BUFFER, N * sizeof(T), zd.data(), GL_STATIC_DRAW);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
        }

        // Copy data in the sm::vec data over to the GPU, once it has been initialized with init_buffer_object
        // data may contain less than N elements, and may be copied at an offset
        template<std::size_t _N = N>
        void copy_to_gpu (sm::vec<T, _N>& _data, std::size_t offset = 0u)
        {
            static_assert (_N <= N);

            // Update local cached version of data, a portion of which we're about to write to the SSBO
            for (unsigned int i = 0; i < _N; ++i) { this->data[i + offset] = _data[i]; }

            this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            T* cpuptr = static_cast<T*>(this->glfn->MapBufferRange (GL_SHADER_STORAGE_BUFFER,
                                                                    offset * sizeof(T), _N * sizeof(T), GL_MAP_WRITE_BIT));
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            // Note: cpuptr is a 'pointer to the beginning of the mapped range' and so we don't incorporate offset again, below:
            for (unsigned int i = 0; i < _N; ++i) { cpuptr[i] = _data[i]; }
            this->glfn->UnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
        }

        void copy_to_gpu()
        {
            this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            T* cpuptr = static_cast<T*>(this->glfn->MapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, N * sizeof(T), GL_MAP_WRITE_BIT));
            if (cpuptr == nullptr) {
                // Error
                std::cout << "ssbo::copy_to_gpu(): MapBufferRange error" << std::endl;
                mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            } else {
                // Note: cpuptr is a 'pointer to the beginning of the mapped range' and so we don't incorporate offset again, below:
                for (unsigned int i = 0; i < N; ++i) { cpuptr[i] = this->data[i]; }
                this->glfn->UnmapBuffer (GL_SHADER_STORAGE_BUFFER);
                this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
                mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            }
        }

        // Map the GPU memory to CPU space, then copy the values into data. NB: it's a
        // performance hit to *copy* to the mapped data to the sm::vec, because the data is
        // *already in CPU accessible memory* after glMapBufferRange().
        //
        // The fastest way to compute on the CPU side would be to add a method to this class (or an
        // extension of it) and operate directly on the pointer cpuptr.
        template<std::size_t _N = N>
        void copy_from_gpu (std::size_t offset = 0u)
        {
            static_assert (_N <= N);
            this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            T* cpuptr = static_cast<T*>(this->glfn->MapBufferRange (GL_SHADER_STORAGE_BUFFER,
                                                                    offset * sizeof(T), _N * sizeof(T), GL_MAP_READ_BIT));
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            for (unsigned int i = 0; i < _N; ++i) { this->data[i + offset] = cpuptr[i]; }
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
            T* cpuptr = static_cast<T*>(this->glfn->MapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, N * sizeof(T), GL_MAP_READ_BIT));
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            for (unsigned int i = 0; i < N; ++i) { r.update (cpuptr[i]); }
            this->glfn->UnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            return r;
        }
    };

} // mplot::gl
