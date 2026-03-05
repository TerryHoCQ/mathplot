/*!
 * \file
 *
 * Declares a VisualResource class to hold the information about Freetype and any other
 * one-per-program resources.
 *
 * \author Seb James
 * \date November 2020
 */
module;

#include <tuple>
#include <memory>
#include <stdexcept>
#include <array>
#include <map>
#include <limits>

#if defined __gl3_h_ || defined __gl_h_
// GL headers have been externally included
#else
// Include GLAD header
# define GLAD_GL_IMPLEMENTATION
#  include <mplot/glad/gl_mx.h>
#endif

#include <mplot/gl/version.h>
#include <mplot/gl/util_mx.h>
#include <mplot/gl/ssbo_mx.h>

// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

export module mplot.core:visualresources;
import :visualface;
import :visualfont;
import :visualresourcesbase;
import :textfeatures;

import sm.vec;

export namespace mplot
{
    // Pointers to mplot::VisualBase are used to index font faces
    //template<int>
    //class VisualBase;

    //! Singleton resource class for mplot::Visual scenes.
    template <int glver>
    class VisualResources : public VisualResourcesBase<glver>
    {
    private:
        VisualResources(){}
        ~VisualResources() { this->faces.clear(); }

        //! The collection of VisualFaces generated for this instance of the
        //! application. Create one VisualFace for each unique combination of VisualFont
        //! and fontpixels (the texture resolution)
        std::map<std::tuple<mplot::VisualFont, unsigned int, mplot::VisualBase<glver>*>,
                 std::unique_ptr<mplot::visgl::VisualFace>> faces;
    public:
        VisualResources(const VisualResources<glver>&) = delete;
        VisualResources& operator=(const VisualResources<glver> &) = delete;
        VisualResources(VisualResources<glver> &&) = delete;
        VisualResources & operator=(VisualResources<glver> &&) = delete;

        //! Initialize a freetype library instance and add to this->freetypes. I wanted
        //! to have only a single freetype library instance, but this didn't work, so I
        //! create one FT_Library for each OpenGL context (i.e. one for each mplot::Visual
        //! window). Thus, arguably, the FT_Library should be a member of mplot::Visual,
        //! but that's a task for the future, as I coded it this way under the false
        //! assumption that I'd only need one FT_Library.
        void freetype_init (mplot::VisualBase<glver>* _vis, GladGLContext* glfn = nullptr)
        {
            FT_Library freetype = nullptr;
            try {
                freetype = this->freetypes.at (_vis);
            } catch (const std::out_of_range&) {
                // Use of gl calls here may make it neat to set up GL here in VisualResources?
                glfn->PixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
                mplot::gl::Util::checkError (__FILE__, __LINE__, glfn);

                if (FT_Init_FreeType (&freetype)) {
                    std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
                } else {
                    // Successfully initialized freetype
                    this->freetypes[_vis] = freetype;
                }
            }
        }

        //! The instance public function. Uses the very short name 'i' to keep code tidy.
        //! This relies on C++11 magic statics (N2660).
        static auto& i()
        {
            static VisualResources<glver> instance;
            return instance;
        }

        //! A function to call to simply make sure the singleton instance exists
        void create() final {}

        //! Return a pointer to a VisualFace for the given \a font at the given texture
        //! resolution, \a fontpixels and the given window (i.e. OpenGL context) \a _win.
        mplot::visgl::VisualFace* getVisualFace (mplot::VisualFont font, unsigned int fontpixels,
                                                 mplot::VisualBase<glver>* _vis, GladGLContext* glfn)
        {
            mplot::visgl::VisualFace* rtn = nullptr;
            auto key = std::make_tuple(font, fontpixels, _vis);
            try {
                rtn = this->faces.at(key).get();
            } catch (const std::out_of_range&) {
                this->faces[key] = std::make_unique<mplot::visgl::VisualFace> (font, fontpixels, this->freetypes.at(_vis), glfn);
                rtn = this->faces.at(key).get();
            }
            return rtn;
        }

        mplot::visgl::VisualFace* getVisualFace (const mplot::TextFeatures& tf,
                                                 mplot::VisualBase<glver>* _vis, GladGLContext* glfn)
        {
            return this->getVisualFace (tf.font, tf.fontres, _vis, glfn);
        }

        //! Loop through this->faces clearing out those associated with the given mplot::Visual
        void clearVisualFaces (mplot::VisualBase<glver>* _vis) final
        {
            auto f = this->faces.begin();
            while (f != this->faces.end()) {
                // f->first is a key. If its third, Visual<>* element == _vis, then delete and erase
                if (std::get<mplot::VisualBase<glver>*>(f->first) == _vis) {
                    f = this->faces.erase (f);
                } else { f++; }
            }
        }

        uint32_t register_visual (GladGLContext* glfn)
        {
            uint32_t visual_id = this->next_visual_id++;
            visual_gladglcontexts[visual_id] = glfn;
            return visual_id;
        }

        uint32_t next_visual_id = 0;

        // Pointers to Visuals in the program, keyed by a uint32_t ID
        std::map<uint32_t, GladGLContext*> visual_gladglcontexts;

        // A VisualModel can call this, passing in the numeric ID of the context it belongs to and
        // this will pass back the correct GL context pointer.
        GladGLContext* get_glfn (uint32_t visual_id)
        {
            // somehow set context from visual_pointers[visual_id]->setContext();
            GladGLContext* glfn = visual_gladglcontexts[visual_id];
            return glfn;
        }

        /*!
         * We also manage some programm-wide SSBO objects for instanced rendering
         * VisualResourcesdata in . Reserve n_to_reserve instances of data in the SSBOs. Return the
         * start offset into the buffers in terms of number of instances
         */
        unsigned int init_instance_ssbo (GladGLContext* glfn, const unsigned int n_to_reserve)
        {
            unsigned int reservation = std::numeric_limits<unsigned int>::max();
            if constexpr (mplot::gl::version::has_ssbo (glver) == true) {
                if (this->instance_data.ready() == false) { this->instance_data.init (glfn); }
                if (this->instparam_data.ready() == false) { this->instparam_data.init (glfn); }
                if (n_to_reserve + this->instance_top <= this->max_instances) {
                    reservation = this->instance_top;
                    this->instance_top += n_to_reserve;
                    this->instance_data.resize (this->instance_top * this->floats_per_instance);
                    this->instparam_data.resize (this->instance_top * this->floats_per_instparam);
                }
            } else {
                throw std::runtime_error ("Instanced rendering requires OpenGL 4.3 or higher");
            }
            return reservation;
        }

        void insert_instance_data (const unsigned int instance_idx, const sm::vec<float, 3>& coord)
        {
            // If this function fails, make sure to call v.render before calling set_instance_data :)
            if (instance_idx >= this->max_instances) {
                throw std::runtime_error ("insert_instance_data: bad instance_idx");
            }
            unsigned int cur_fidx = instance_idx * this->floats_per_instance;
            this->instance_data.data[cur_fidx++] = coord[0];
            this->instance_data.data[cur_fidx++] = coord[1];
            this->instance_data.data[cur_fidx++] = coord[2];
        }

        void insert_instparam_data (const unsigned int instance_idx,
                                    const std::array<float, 3>& colour, const float& alpha, const float& scale)
        {
            if (instance_idx >= this->max_instances) {
                throw std::runtime_error ("insert_instparam_data: bad instance_idx");
            }
            unsigned int cur_fidx = instance_idx * this->floats_per_instparam;
            this->instparam_data.data[cur_fidx++] = colour[0];
            this->instparam_data.data[cur_fidx++] = colour[1];
            this->instparam_data.data[cur_fidx++] = colour[2];
            this->instparam_data.data[cur_fidx++] = alpha;
            this->instparam_data.data[cur_fidx++] = scale;
        }

        void copy_instance_ssbo_to_gpu()
        {
            if (this->instance_data.ready()) { this->instance_data.copy_to_gpu(); }
            if (this->instparam_data.ready()) { this->instparam_data.copy_to_gpu(); }
        }

        //! Shader Storage Buffer Object for instanced rendering - this holds positions only
        mplot::gl::ssbo<mplot::VisualResourcesBase<glver>::instance_index,
                        float, mplot::VisualResourcesBase<glver>::max_instance_floats> instance_data;
        //! Shader Storage Buffer Object for instanced rendering - this holds colour, alpha and scale
        mplot::gl::ssbo<mplot::VisualResourcesBase<glver>::instparam_index,
                        float, mplot::VisualResourcesBase<glver>::max_instparam_floats> instparam_data;
    };

} // namespace mplot
