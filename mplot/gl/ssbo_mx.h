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
    template <unsigned int index, typename T, std::size_t N> // Could add version template params if necessary, to select correct gl function calls
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

    // Set up a Shader Storage Buffer Object (SSBO) and buffer data into it (from a sm::vvec)
    template<typename T>
    void setup_ssbo (const GLuint target_index, unsigned int& ssbo_id, const sm::vvec<T>& data)
    {
        this->glfn->GenBuffers (1, &ssbo_id);
        this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, target_index, ssbo_id);
        // Mutable, re-locatable storage:
        this->glfn->BufferData (GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
        // Immutable storage would be:
        // void glBufferStorage(GLenum target​, GLsizeiptr size​, const GLvoid * data​, GLbitfield flags​);
        //this->glfn->BufferStorage (GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_CLIENT_STORAGE_BIT | GL_MAP_READ_BIT);
        this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
        mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
    }

    // Set up a Shader Storage Buffer Object (SSBO) and buffer data into it (sm::vec version)
    template<typename T, unsigned int N>
    void setup_ssbo (const GLuint target_index, unsigned int& ssbo_id, const sm::vec<T, N>& data)
    {
        this->glfn->GenBuffers (1, &ssbo_id);
        this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, target_index, ssbo_id);
        this->glfn->BufferData (GL_SHADER_STORAGE_BUFFER, N * sizeof(T), data.data(), GL_STATIC_DRAW);
        this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
        mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
    }

    // Copy data to an existing SSBO
    template<typename T>
    void copy_vvec_to_ssbo (const GLuint target_index, const unsigned int ssbo_id, const sm::vvec<T>& data)
    {
        this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, target_index, ssbo_id);
        this->glfn->BufferData (GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
        this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
        mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
    }

    template<typename T, unsigned int N>
    void copy_vvec_to_ssbo (const GLuint target_index, const unsigned int ssbo_id, const sm::vvec<T>& data)
    {
        this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, target_index, ssbo_id);
        this->glfn->BufferData (GL_SHADER_STORAGE_BUFFER, N * sizeof(T), data.data(), GL_STATIC_DRAW);
        this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
        mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
    }

    // Map the SSBO to cpu space, then make a copy of the data into a passed-in vvec.
    //
    // ssbo_idx: The Index of the Shader Storage Buffer Object that we're reading from
    // ssbo_name: The name (really a number) of the Shader Storage Buffer Object that we're reading from
    // cpu_data: A vvec of the right size to receive the data in the SSBO into 'CPU accessible memory'
    template <typename T>
    void ssbo_copy_to_vvec (const unsigned int ssbo_idx, const unsigned int ssbo_name, sm::vvec<T>& cpu_side)
    {
        this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, ssbo_idx, ssbo_name);
        // Really, it's crazy to *copy* because the data is *already in CPU
        // accessible memory* after glMapBufferRange. BUT here's the copy:
        T* cpuptr = static_cast<T*>(this->glfn->MapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, cpu_side.size()*sizeof(T), GL_MAP_READ_BIT));
        for (unsigned int i = 0; i < cpu_side.size(); ++i) { cpu_side[i] = cpuptr[i]; }
        this->glfn->UnmapBuffer (GL_SHADER_STORAGE_BUFFER);
        this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
        mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
    }

    template <typename T, unsigned int N>
    void ssbo_copy_to_vec (const unsigned int ssbo_idx, const unsigned int ssbo_name, sm::vec<T, N>& cpu_side)
    {
        this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, ssbo_idx, ssbo_name);
        // Really, it's crazy to *copy* because the data is *already in CPU
        // accessible memory* after glMapBufferRange. BUT here's the copy:
        T* cpuptr = static_cast<T*>(this->glfn->MapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, N*sizeof(T), GL_MAP_READ_BIT));
        for (unsigned int i = 0; i < N; ++i) { cpu_side[i] = cpuptr[i]; }
        this->glfn->UnmapBuffer (GL_SHADER_STORAGE_BUFFER);
        this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
        mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
    }

    // Find the range of the data in the given Shader Storage Buffer Object
    //
    // ssbo_idx: The Index of the Shader Storage Buffer Object that we're reading from
    // ssbo_name: The name (really a number) of the Shader Storage Buffer Object that we're reading from
    // ssbo_num_elements: The number of elements of type T in the SSBO.
    template <typename T>
    sm::range<T> ssbo_get_range (const unsigned int ssbo_idx, const unsigned int ssbo_name, const unsigned int ssbo_num_elements)
    {
        sm::range<T> r;
        r.search_init();
        this->glfn->BindBufferBase (GL_SHADER_STORAGE_BUFFER, ssbo_idx, ssbo_name);
        T* cpuptr = static_cast<T*>(this->glfn->MapBufferRange (GL_SHADER_STORAGE_BUFFER, 0, ssbo_num_elements*sizeof(T), GL_MAP_READ_BIT));
        for (unsigned int i = 0; i < ssbo_num_elements; ++i) { r.update (cpuptr[i]); }
        this->glfn->UnmapBuffer (GL_SHADER_STORAGE_BUFFER);
        this->glfn->BindBuffer (GL_SHADER_STORAGE_BUFFER, 0);
        mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
        return r;
    }

} // mplot::gl
