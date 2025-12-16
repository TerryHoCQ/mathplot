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
    // @tparam N: The max number of elements of type T in the SSBO
    template <unsigned int index, typename T, std::size_t N> // Could add version template params if necessary, to select correct gl function calls
    struct ssbo
    {
        // The name of the buffer, generated with glGenBuffers()
        unsigned int name = 0;
        // The CPU-side data for the buffer
        sm::vvec<T> data = {};

        bool ready() const { return this->name != 0u; }

        ssbo() {}
        ~ssbo() {}

        // Init is not built into the constructor, as client code must ensure there is an OpenGL context available
        void init()
        {
            glGenBuffers (1, &this->name);
            this->data.reserve (N * sizeof(T));
            this->init_buffer_object();
        }

        void init_buffer_object()
        {
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            glBufferData (GL_SHADER_STORAGE_BUFFER, N * sizeof(T), this->data.data(), GL_STATIC_DRAW);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
        }

        // Copy data in the sm::vec data over to the GPU, once it has been initialized with init_buffer_object
        // data may contain less than N elements, and may be copied at an offset
        template<std::size_t _N = N>
        void copy_to_gpu (sm::vec<T, _N>& _data, std::size_t offset = 0u)
        {
            static_assert (_N <= N);

            // Update local cached version of data, a portion of which we're about to write to the SSBO
            this->data.resize (_N);
            for (unsigned int i = 0; i < _N; ++i) { this->data[i + offset] = _data[i]; }

            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, offset * sizeof(T), _N * sizeof(T), GL_MAP_WRITE_BIT));
            if (cpuptr == nullptr) {
                std::cout << "ssbo::copy_to_gpu(sm::vec<T, _N>&): MapBufferRange error" << std::endl;
                mplot::gl::Util::checkError (__FILE__, __LINE__);
                return;
            }
            // Note: cpuptr is a 'pointer to the beginning of the mapped range' and so we don't incorporate offset again, below:
            for (unsigned int i = 0; i < _N; ++i) { cpuptr[i] = _data[i]; }
            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
        }

        void copy_to_gpu (sm::vvec<T>& _data, std::size_t offset = 0u)
        {
            if (_data.size() > N) { throw std::runtime_error ("Too big"); }

            // Update local cached version of data, a portion of which we're about to write to the SSBO
            this->data.resize (_data.size());
            for (unsigned int i = 0; i < _data.size(); ++i) { this->data[i + offset] = _data[i]; }

            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            std::cout << "Mapping buffer range at offset " << offset << " floats = " << (offset * sizeof(T)) << " bytes" << std::endl;
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER,
                                                          offset * sizeof(T), _data.size() * sizeof(T),
                                                          GL_MAP_WRITE_BIT));
            if (cpuptr == nullptr) {
                std::cout << "ssbo::copy_to_gpu(sm::vec<T, _N>&): MapBufferRange error" << std::endl;
                mplot::gl::Util::checkError (__FILE__, __LINE__);
                return;
            }
            // Note: cpuptr is a 'pointer to the beginning of the mapped range' and so we don't incorporate offset again, below:
            std::cout << "About to copy " << _data.size() << " things (prolly floats)...\n";
            for (unsigned int i = 0; i < _data.size(); ++i) {
                std::cout << "Writing value " << " _data[i] = " << _data[i] << " into mapped range" << std::endl;
                cpuptr[i] = _data[i];
            }

            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);    // necessary
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);  // optional
            mplot::gl::Util::checkError (__FILE__, __LINE__);
        }

        void copy_to_gpu ()
        {
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            std::size_t sz = this->data.size();
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, sz * sizeof(T), GL_MAP_WRITE_BIT));
            if (cpuptr == nullptr) {
                std::cout << "ssbo::copy_to_gpu(): MapBufferRange error" << std::endl;
                mplot::gl::Util::checkError (__FILE__, __LINE__);
                return;
            }
            // Note: cpuptr is a 'pointer to the beginning of the mapped range' and so we don't incorporate offset again, below:
            for (unsigned int i = 0; i < sz; ++i) { cpuptr[i] = this->data[i]; }
            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
        }

        // Map the GPU memory to CPU space, then copy the values into this->data. NB: it's a
        // performance hit to *copy* to the mapped data to our sm::vec, because the data is
        // *already in CPU accessible memory* after glMapBufferRange().
        // However, in case you need it, here it is.
        template<std::size_t _N = N>
        void copy_from_gpu (std::size_t offset = 0u)
        {
            static_assert (_N <= N);
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, offset * sizeof(T), _N * sizeof(T), GL_MAP_READ_BIT));
            if (cpuptr == nullptr) {
                std::cout << "ssbo::copy_from_gpu(): MapBufferRange error" << std::endl;
                mplot::gl::Util::checkError (__FILE__, __LINE__);
                return;
            }
            this->data.resize (_N);
            for (unsigned int i = 0; i < _N; ++i) { data[i + offset] = cpuptr[i]; }
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
            std::size_t sz = this->data.size();
            sm::range<T> r;
            r.search_init();
            glBindBufferBase (GL_SHADER_STORAGE_BUFFER, index, this->name);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            T* cpuptr = static_cast<T*>(glMapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, sz * sizeof(T), GL_MAP_READ_BIT));
            if (cpuptr == nullptr) {
                std::cout << "ssbo::get_range(): MapBufferRange error" << std::endl;
                mplot::gl::Util::checkError (__FILE__, __LINE__);
                return r;
            }
            for (unsigned int i = 0; i < N; ++i) { r.update (cpuptr[i]); }
            glUnmapBuffer (GL_SHADER_STORAGE_BUFFER);
            glBindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
            mplot::gl::Util::checkError (__FILE__, __LINE__);
            return r;
        }
    };

} // mplot::gl
