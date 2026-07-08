/*!
 * \file
 *
 * Declares a VisualResource class to hold the information about Freetype which is a one-per-program
 * resource. Also holds the Glad contexts - the OpenGL function pointers for each window. Windows
 * are referred to as mplot::win_t; the identity of win_t (which might be GLFWwindow or
 * QtOpenGLWidget) is abstact in this file.
 *
 * \author Seb James
 * \date November 2020
 */
module;

// Include GLAD header
#include <mplot/glad/gl.h>

// FreeType for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <functional>
#include <mplot/gl/ssbo_mx.h>

export module mplot.visualresources;

import mplot.visualcommon;
import mplot.visualface;
import mplot.visualfont;
import mplot.textfeatures;
import mplot.win_t;
import mplot.gl.version;
import mplot.gl.util;
import sm.vec;

export namespace mplot
{
    //! Singleton resource class for mplot::Visual scenes.
    template <std::int32_t glver>
    class VisualResources
    {
    private:
        VisualResources(){}
        ~VisualResources()
        {
            this->faces.clear();
            // As with the case for faces, when each mplot::Visual goes out of scope, the FreeType
            // instance gets cleaned up. So at this stage freetypes should also be empy and nothing
            // will happen here either.
            for (auto& ft : this->freetypes) { FT_Done_FreeType (ft.second); }
        }

        //! The collection of VisualFaces generated for this instance of the application. Create one
        //! VisualFace for each unique combination of VisualFont and fontpixels (the texture
        //! resolution). uint32_t is the 'visual_id' an ID of the Visual instance.
        std::map<std::tuple<mplot::VisualFont, std::uint32_t, std::uint32_t>,
                 std::unique_ptr<mplot::visgl::VisualFace>> faces;

        //! FreeType library object. Keyed by the 'visual_id' an ID of the Visual instance.
        std::map<std::uint32_t, FT_Library> freetypes;

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
        void freetype_init (std::uint32_t _vis, GladGLContext* glfn = nullptr)
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

        //! When a mplot::Visual goes out of scope, its freetype library instance should be
        //! deinitialized.
        void freetype_deinit (std::uint32_t _vis)
        {
            // First clear the faces associated with Visual with ID _vis
            this->clearVisualFaces (_vis);
            // Second, clean up the FreeType library instance and erase from this->freetypes
            auto freetype = this->freetypes.find (_vis);
            if (freetype != this->freetypes.end()) {
                FT_Done_FreeType (freetype->second);
                this->freetypes.erase (freetype);
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
        void create() {}

        //! Return a pointer to a VisualFace for the given \a font at the given texture
        //! resolution, \a fontpixels and the given window (i.e. OpenGL context) \a _win.
        mplot::visgl::VisualFace* getVisualFace (mplot::VisualFont font, std::uint32_t fontpixels,
                                                 std::uint32_t _vis, GladGLContext* glfn)
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
                                                 const std::uint32_t _vis, GladGLContext* glfn)
        {
            return this->getVisualFace (tf.font, tf.fontres, _vis, glfn);
        }

        //! Loop through this->faces clearing out those associated with the given mplot::Visual
        void clearVisualFaces (const std::uint32_t _vis)
        {
            mplot::VisualFont thefont;
            std::uint32_t fpixels = 0;
            std::uint32_t vf_vis = std::numeric_limits<std::uint32_t>::max();

            auto f = this->faces.begin();

            while (f != this->faces.end()) {
                // f->first is a key. If its third, visual_id element == _vis, then delete and erase
                // f->first needs unpacking; want 3rd elemetn
                std::tie(thefont, fpixels, vf_vis) = f->first;

                if (vf_vis == _vis) {
                    f = this->faces.erase (f);
                } else { f++; }
            }
        }

        std::uint32_t next_visual_id = 0;

        // GL function context pointers used in the program, keyed by a uint32_t ID
        std::map<std::uint32_t, GladGLContext*> visual_keyed_gladglcontexts;
        // GL shader programs used by Visual in the program, keyed by ID
        std::map<std::uint32_t, mplot::visgl::visual_shaderprogs> visual_keyed_shaderprogs;
        // Window contexts
        std::map<std::uint32_t, mplot::win_t*> visual_keyed_windows;
        // Does instanced data need update?
        std::map<std::uint32_t, bool> visual_keyed_instanced_needs_update;

        std::uint32_t register_visual (GladGLContext* glfn, mplot::win_t* win)
        {
            std::uint32_t visual_id = this->next_visual_id++;
            this->visual_keyed_gladglcontexts[visual_id] = glfn;
            this->visual_keyed_shaderprogs[visual_id] = {}; // initialized empty with 0s
            this->visual_keyed_windows[visual_id] = win;
            this->visual_keyed_instanced_needs_update[visual_id] = false;
            return visual_id;
        }

        // Return true if there is a GladGLContext for visual_id
        bool test_glfn (const std::uint32_t visual_id)
        {
            if (visual_id == std::numeric_limits<std::uint32_t>::max()) {
                return false;
            }
            try {
                [[maybe_unused]] GladGLContext* glfn = this->visual_keyed_gladglcontexts.at (visual_id);
                return true;
            } catch (const std::exception& e) {}
            return false;
        }

        /*!
         * A VisualModel can call this method, passing in the numeric ID of the context it belongs
         * to. The return value will be the correct GL context pointer.
         */
        GladGLContext* get_glfn (const std::uint32_t visual_id) noexcept
        {
            if (visual_id == std::numeric_limits<std::uint32_t>::max()) { return nullptr; }
            GladGLContext* glfn = this->visual_keyed_gladglcontexts[visual_id];
            return glfn;
        }

        void set_tprog (const std::uint32_t visual_id, const std::uint32_t _tprog)
        {
            if (visual_id == std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error ("VisualResources::set_tprog(): visual_id is unset");
            }
            this->visual_keyed_shaderprogs[visual_id].tprog = _tprog;
        }

        std::uint32_t get_tprog (const std::uint32_t visual_id)
        {
            if (visual_id == std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error ("VisualResources::get_tprog(): visual_id is unset");
            }
            return this->visual_keyed_shaderprogs[visual_id].tprog;
        }

        void set_gprog (const std::uint32_t visual_id, const std::uint32_t _gprog)
        {
            if (visual_id == std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error ("VisualResources::set_gprog(): visual_id is unset");
            }
            this->visual_keyed_shaderprogs[visual_id].gprog = _gprog;
        }

        std::uint32_t get_gprog (const std::uint32_t visual_id)
        {
            if (visual_id == std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error ("VisualResources::get_gprog(): visual_id is unset");
            }
            return this->visual_keyed_shaderprogs[visual_id].gprog;
        }

        /*!
         * Set the window context. This has to be window-system agnostic here, so we have the
         * std::function setContext_implementation that (if you are using GLFW windows) is linked at
         * runtime to VisualGlfw::setContext(GLFWwindow*)
         */
        void setContext (const std::uint32_t visual_id)
        {
            if (visual_id == std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error ("VisualResources::setContext(): visual_id is unset");
            }
            if (this->setContext_implementation) {
                this->setContext_implementation (this->visual_keyed_windows[visual_id]);
            }
        }

        //! setContext function call object
        std::function<void(mplot::win_t*)> setContext_implementation;

        //! Window-system agnostic releaseContext call (see also setContext)
        void releaseContext() { if (this->releaseContext_implementation) { this->releaseContext_implementation(); } }

        //! releaseContext function call object
        std::function<void()> releaseContext_implementation;

        bool get_instanced_needs_update (const std::uint32_t visual_id)
        {
            if (visual_id == std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error ("VisualResources::get_instanced_needs_update: visual_id is unset");
            }
            return this->visual_keyed_instanced_needs_update[visual_id];
        }

        void instanced_needs_update (const std::uint32_t visual_id, const bool val = true)
        {
            if (visual_id == std::numeric_limits<std::uint32_t>::max()) {
                throw std::runtime_error ("VisualResources::instanced_needs_update: visual_id is unset");
            }
            this->visual_keyed_instanced_needs_update[visual_id] = val;
        }

        /*!
         * We also manage some programm-wide SSBO objects for instanced rendering
         * VisualResourcesdata in . Reserve n_to_reserve instances of data in the SSBOs. Return the
         * start offset into the buffers in terms of number of instances
         */
        std::uint32_t init_instance_ssbo (const std::uint32_t visual_id, const std::uint32_t n_to_reserve)
        {
            GladGLContext* glfn = this->get_glfn (visual_id);
            std::uint32_t reservation = std::numeric_limits<std::uint32_t>::max();
            if constexpr (mplot::gl::version::has_ssbo (glver) == true) {
                if (this->instance_data.ready() == false) { this->instance_data.init (glfn); }
                if (this->instparam_data.ready() == false) { this->instparam_data.init (glfn); }
                if (n_to_reserve + this->instance_top <= this->max_instances) {
                    reservation = this->instance_top;
                    this->instance_top += n_to_reserve;
                    this->instance_data.resize (this->instance_top * this->floats_per_instance, 1000.0f);
                    this->instparam_data.resize (this->instance_top * this->floats_per_instparam);
                }
            } else {
                throw std::runtime_error ("Instanced rendering requires OpenGL 4.3 or higher");
            }
            return reservation;
        }

        /*!
         * Inserts the 3 values from coord into the SSBO instance buffer, starting at location
         * instance_idx
         */
        void insert_instance_data (const std::uint32_t instance_idx, const sm::vec<float, 3>& coord)
        {
            // If this function fails, make sure to call v.render before calling set_instance_data :)
            if (instance_idx >= this->max_instances) {
                throw std::runtime_error ("insert_instance_data: bad instance_idx");
            }
            std::uint32_t cur_fidx = instance_idx * this->floats_per_instance;
            this->instance_data.data[cur_fidx++] = coord[0];
            this->instance_data.data[cur_fidx++] = coord[1];
            this->instance_data.data[cur_fidx++] = coord[2];
        }

        /*!
         * Inserts the 3 values from colour, along with alpha and scale into the SSBO instparam
         * buffer, starting at location instance_idx
         */
        void insert_instparam_data (const std::uint32_t instance_idx,
                                    const std::array<float, 3>& colour, const float& alpha, const float& scale)
        {
            if (instance_idx >= this->max_instances) {
                throw std::runtime_error ("insert_instparam_data: bad instance_idx");
            }
            std::uint32_t cur_fidx = instance_idx * this->floats_per_instparam;
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

        /*!
         * SSBO management
         */
        //! Instanced rendering mode (SSBO access). position data stored in SSBO index 1 (must match GLSL code)
        static constexpr std::uint32_t instance_index = 1u;
        //! colour, scale, rotation stored in SSBO index 2
        static constexpr std::uint32_t instparam_index = 2u;
        //! one 3D vector is 3 floats
        static constexpr std::uint32_t floats_per_instance = 3u;
        //! Instance params are: colour/alpha (4 floats), scale (1 float)
        static constexpr std::uint32_t floats_per_instparam = 5u;

        /*!
         * This will control how much GPU RAM is allocated when using instanced rendering
         * (Hopefully, when I'm finished, the RAM will be allocated only if at least one VisualModel
         * is marked 'instanced'). Makes a big difference to speed of operation (unless I can send a
         * portion of a buffer to the GPU).
         */
        static constexpr std::uint32_t max_instances = 32u * 1024u;
        static constexpr std::uint32_t max_instance_floats = floats_per_instance * max_instances;
        static constexpr std::uint32_t max_instparam_floats = floats_per_instparam * max_instances;

        //! Shader Storage Buffer Object for instanced rendering - this holds positions only
        mplot::gl::ssbo<mplot::VisualResources<glver>::instance_index,
                        float, mplot::VisualResources<glver>::max_instance_floats> instance_data;
        //! Shader Storage Buffer Object for instanced rendering - this holds colour, alpha and scale
        mplot::gl::ssbo<mplot::VisualResources<glver>::instparam_index,
                        float, mplot::VisualResources<glver>::max_instparam_floats> instparam_data;

        // The Current location from which space in the instance SSBOs should be allocated
        std::uint32_t instance_top = 0;
    };

} // namespace mplot
