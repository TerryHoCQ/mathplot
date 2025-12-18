/*!
 * \file
 *
 * Declares a VisualResource class to hold the information about Freetype and any other
 * one-per-program resources.
 *
 * \author Seb James
 * \date November 2020
 */

#pragma once

#include <mplot/VisualFaceMX.h>
#include <mplot/VisualResourcesBase.h>
#include <mplot/gl/util_mx.h>
#include <mplot/gl/ssbo_mx.h>

namespace mplot
{
    // Pointers to mplot::VisualBase are used to index font faces
    template<int>
    class VisualBase;

    //! Singleton resource class for mplot::Visual scenes.
    template <int glver>
    class VisualResourcesMX : public VisualResourcesBase<glver>
    {
    private:
        VisualResourcesMX(){}
        ~VisualResourcesMX() { this->faces.clear(); }

        //! The collection of VisualFaces generated for this instance of the
        //! application. Create one VisualFace for each unique combination of VisualFont
        //! and fontpixels (the texture resolution)
        std::map<std::tuple<mplot::VisualFont, unsigned int, mplot::VisualBase<glver>*>,
                 std::unique_ptr<mplot::visgl::VisualFaceMX>> faces;
    public:
        VisualResourcesMX(const VisualResourcesMX<glver>&) = delete;
        VisualResourcesMX& operator=(const VisualResourcesMX<glver> &) = delete;
        VisualResourcesMX(VisualResourcesMX<glver> &&) = delete;
        VisualResourcesMX & operator=(VisualResourcesMX<glver> &&) = delete;

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
            static VisualResourcesMX<glver> instance;
            return instance;
        }

        //! A function to call to simply make sure the singleton instance exists
        void create() final {}

        //! Return a pointer to a VisualFace for the given \a font at the given texture
        //! resolution, \a fontpixels and the given window (i.e. OpenGL context) \a _win.
        mplot::visgl::VisualFaceMX* getVisualFace (mplot::VisualFont font, unsigned int fontpixels,
                                                   mplot::VisualBase<glver>* _vis, GladGLContext* glfn)
        {
            mplot::visgl::VisualFaceMX* rtn = nullptr;
            auto key = std::make_tuple(font, fontpixels, _vis);
            try {
                rtn = this->faces.at(key).get();
            } catch (const std::out_of_range&) {
                this->faces[key] = std::make_unique<mplot::visgl::VisualFaceMX> (font, fontpixels, this->freetypes.at(_vis), glfn);
                rtn = this->faces.at(key).get();
            }
            return rtn;
        }

        mplot::visgl::VisualFaceMX* getVisualFace (const mplot::TextFeatures& tf,
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
                }
            } else {
                throw std::runtime_error ("Instanced rendering requires OpenGL 4.3 or higher");
            }
            return reservation;
        }

        void insert_instance_data (const unsigned int instance_idx, const sm::vec<float, 3>& coord)
        {
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

        //! Shader Storage Buffer Object for instanced rendering - this holds positions only
        mplot::gl::ssbo<mplot::VisualResourcesBase<glver>::instance_index,
                        float, mplot::VisualResourcesBase<glver>::max_instance_floats> instance_data;
        //! Shader Storage Buffer Object for instanced rendering - this holds colour, alpha and scale
        mplot::gl::ssbo<mplot::VisualResourcesBase<glver>::instparam_index,
                        float, mplot::VisualResourcesBase<glver>::max_instparam_floats> instparam_data;

    };

} // namespace mplot
