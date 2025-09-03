/*!
 * \file
 *
 * Awesome graphics code for high performance graphing and visualisation.
 *
 * This is intermediate class that sets up GL, leaving choice of window system (GLFW3/Qt/wx/etc) to
 * a derived class such as mplot::Visual or mplot::qt::viswidget.
 *
 * This class is 'ownable', and can be used in other window drawing system such as Qt and wx, as
 * well as within mplot::Visual[NoMx], which marries it with the GLFW3 windowing system.
 *
 * Created by Seb James on 2025/03/01, from mplot::Visual.h
 *
 * \author Seb James
 * \date March 2025
 */
#pragma once

#if defined __gl3_h_ || defined __gl_h_ // could get a fuller list from glfw.h
// GL headers appear to have been externally included.
#else
// Include GLAD header
# define GLAD_GL_IMPLEMENTATION
#  include <mplot/glad/gl.h>
#endif // GL headers

// By including this header, you take out a contract that you are not using multicontext (MX) GLAD
// headers. This must appear BEFORE the rest of the mplot headers.
namespace mplot { namespace gl { static constexpr int multicontext = 0; } }

#include <mplot/VisualResourcesNoMX.h>
#include <mplot/VisualTextModel.h>
#include <mplot/VisualBase.h>
#include <mplot/gl/loadshaders_nomx.h>
#include <mplot/gl/util_nomx.h>

namespace mplot {

    /*!
     * VisualOwnableNoMX - adds GL calls to the 'scene' base class, VisualBase
     *
     * This class assumes that GL functions have been loaded as global aliases, such as glClear()
     * for Clear() and glEnable() for Enable().
     *
     * \tparam glver The OpenGL version, encoded as a single int (see mplot::gl::version)
     */
    template <int glver = mplot::gl::version_4_1>
    class VisualOwnableNoMX : public mplot::VisualBase<glver>
    {
    public:
        /*!
         * Default constructor is used when incorporating Visual inside another object
         * such as a QWidget.  We have to wait on calling init functions until an OpenGL
         * environment is guaranteed to exist.
         */
        VisualOwnableNoMX() {}

        /*!
         * Construct a new visualiser. The rule is 1 window to one Visual object. So, this creates a
         * new window and a new OpenGL context.
         */
        VisualOwnableNoMX (const int _width, const int _height, const std::string& _title, const bool _version_stdout = true)
        {
            this->window_w = _width;
            this->window_h = _height;
            this->title = _title;
            this->options.set (visual_options::versionStdout, _version_stdout);

            this->init_gl();
        }

        //! Deconstruct gl memory/context
        void deconstructCommon()
        {
            // Explicitly deconstruct any owned VisualModels
            this->vm.clear();
            // Explicitly deconstruct coordArrows, textModel and texts here
            this->coordArrows.reset (nullptr);
            this->userFrame.reset (nullptr);
            this->textModel.reset (nullptr);
            for (auto& t : this->texts) { t.reset (nullptr); }

            if (this->shaders.gprog) {
                glDeleteProgram (this->shaders.gprog);
                this->shaders.gprog = 0;
                this->active_gprog = mplot::visgl::graphics_shader_type::none;
            }
            if (this->shaders.tprog) {
                glDeleteProgram (this->shaders.tprog);
                this->shaders.tprog = 0;
            }
            // Free up the Fonts associated with this mplot::Visual
            mplot::VisualResourcesNoMX<glver>::i().freetype_deinit (this);
        }

    protected:
        void freetype_init() final
        {
            // Now make sure that Freetype is set up (we assume that caller code has set the correct OpenGL context)
            mplot::VisualResourcesNoMX<glver>::i().freetype_init (this);
        }

    public:
        // Do one-time init of the Visual's resources. This gets/creates the VisualResources,
        // registers this visual with resources, calls init_window for any glfw stuff that needs to
        // happen, and lastly initializes the freetype code.
        void init_resources()
        {
            // VisualResources provides font management and GLFW management. Ensure it exists in memory.
            mplot::VisualResourcesNoMX<glver>::i().create();
            this->freetype_init();
        }

        //! Stores the OpenGL function context version that was loaded
        int glfn_version = 0;

        //! Take a screenshot of the window. Return vec containing width * height or {-1, -1} on
        //! failure. Set transparent_bg to get a transparent background.
        sm::vec<int, 2> saveImage (const std::string& img_filename, const bool transparent_bg = false)
        {
            this->setContext();

            GLint viewport[4]; // current viewport
            glGetIntegerv (GL_VIEWPORT, viewport);

            sm::vec<int, 2> dims;
            dims[0] = viewport[2];
            dims[1] = viewport[3];
            auto bits = std::make_unique<GLubyte[]>(dims.product() * 4);
            auto rbits = std::make_unique<GLubyte[]>(dims.product() * 4);

            glFinish(); // finish all commands of OpenGL
            glPixelStorei (GL_PACK_ALIGNMENT, 1);
            glPixelStorei (GL_PACK_ROW_LENGTH, 0);
            glPixelStorei (GL_PACK_SKIP_ROWS, 0);
            glPixelStorei (GL_PACK_SKIP_PIXELS, 0);
            glReadPixels (0, 0, dims[0], dims[1], GL_RGBA, GL_UNSIGNED_BYTE, bits.get());

            for (int i = 0; i < dims[1]; ++i) {
                int rev_line = (dims[1] - i - 1) * 4 * dims[0];
                int for_line = i * 4 * dims[0];
                if (transparent_bg) {
                    for (int j = 0; j < 4 * dims[0]; ++j) {
                        rbits[rev_line + j] = bits[for_line + j];
                    }
                } else {
                    for (int j = 0; j < 4 * dims[0]; ++j) {
                        rbits[rev_line + j] = (j % 4 == 3) ? 255 : bits[for_line + j];
                    }
                }
            }
            unsigned int error = lodepng::encode (img_filename, rbits.get(), dims[0], dims[1]);
            if (error) {
                std::cerr << "encoder error " << error << ": " << lodepng_error_text (error) << std::endl;
                dims.set_from (-1);
                return dims;
            }
            return dims;
        }

        //! Render the scene
        void render() noexcept final
        {
            this->setContext();

            if (this->ptype == perspective_type::orthographic || this->ptype == perspective_type::perspective) {
                if (this->active_gprog != mplot::visgl::graphics_shader_type::projection2d) {
                    if (this->shaders.gprog) { glDeleteProgram (this->shaders.gprog); }
                    this->shaders.gprog = mplot::gl::LoadShaders (this->proj2d_shader_progs);
                    this->active_gprog = mplot::visgl::graphics_shader_type::projection2d;
                }
            } else if (this->ptype == perspective_type::cylindrical) {
                if (this->active_gprog != mplot::visgl::graphics_shader_type::cylindrical) {
                    if (this->shaders.gprog) { glDeleteProgram (this->shaders.gprog); }
                    this->shaders.gprog = mplot::gl::LoadShaders (this->cyl_shader_progs);
                    this->active_gprog = mplot::visgl::graphics_shader_type::cylindrical;
                }
            }

            glUseProgram (this->shaders.gprog);
            glViewport (0, 0, this->window_w * mplot::retinaScale, this->window_h * mplot::retinaScale);

            // Set the perspective
            if (this->ptype == perspective_type::orthographic) {
                this->setOrthographic();
            } else if (this->ptype == perspective_type::perspective) {
                this->setPerspective();
            } else if (this->ptype == perspective_type::cylindrical) {
                // Set cylindrical-specific uniforms
                GLint loc_campos = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("cyl_cam_pos"));
                if (loc_campos != -1) { glUniform4fv (loc_campos, 1, this->cyl_cam_pos.data()); }
                GLint loc_cyl_radius = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("cyl_radius"));
                if (loc_cyl_radius != -1) { glUniform1f (loc_cyl_radius, this->cyl_radius); }
                GLint loc_cyl_height = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("cyl_height"));
                if (loc_cyl_height != -1) { glUniform1f (loc_cyl_height, this->cyl_height); }
            } else {
                // unknown projection
                return;
            }

            // Calculate model view transformation - transforming from "model space" to "worldspace".
            this->computeSceneview();

            // Clear color buffer and **also depth buffer**
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set the background colour:
            glClearBufferfv (GL_COLOR, 0, this->bgcolour.data());

            // Lighting shader variables
            //
            // Ambient light colour
            GLint loc_lightcol = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("light_colour"));
            if (loc_lightcol != -1) { glUniform3fv (loc_lightcol, 1, this->light_colour.data()); }
            // Ambient light intensity
            GLint loc_ai = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("ambient_intensity"));
            if (loc_ai != -1) { glUniform1f (loc_ai, this->ambient_intensity); }
            // Diffuse light position
            GLint loc_dp = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("diffuse_position"));
            if (loc_dp != -1) { glUniform3fv (loc_dp, 1, this->diffuse_position.data()); }
            // Diffuse light intensity
            GLint loc_di = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("diffuse_intensity"));
            if (loc_di != -1) { glUniform1f (loc_di, this->diffuse_intensity); }

            // Switch to text shader program and set the projection matrix
            glUseProgram (this->shaders.tprog);
            GLint loc_p = glGetUniformLocation (this->shaders.tprog, static_cast<const GLchar*>("p_matrix"));
            if (loc_p != -1) { glUniformMatrix4fv (loc_p, 1, GL_FALSE, this->projection.mat.data()); }

            // Switch back to the regular shader prog and render the VisualModels.
            glUseProgram (this->shaders.gprog);

            // Set the projection matrix just once
            loc_p = glGetUniformLocation (this->shaders.gprog, static_cast<const GLchar*>("p_matrix"));
            if (loc_p != -1) { glUniformMatrix4fv (loc_p, 1, GL_FALSE, this->projection.mat.data()); }

            if ((this->ptype == perspective_type::orthographic || this->ptype == perspective_type::perspective)
                &&  this->options.test (visual_options::showCoordArrows)) {
                // Ensure coordarrows centre sphere will be visible on BG:
                this->coordArrows->setColourForBackground (this->bgcolour); // releases context...
                this->setContext(); // ...so re-acquire if we're managing it

                if (this->options.test (visual_options::coordArrowsInScene) == true) {
                    this->coordArrows->setSceneMatrix (this->sceneview);
                } else {
                    this->positionCoordArrows();
                }
                this->coordArrows->render();
            }

            // Optional user frame of reference
            if (this->options.test (visual_options::showUserFrame) && this->userFrame) {
                this->userFrame->render();
            }

            auto vmi = this->vm.begin();
            while (vmi != this->vm.end()) {
                if ((*vmi)->twodimensional() == true) {
                    // It's a two-d thing. Now what?
                    (*vmi)->setSceneMatrix (this->sceneview_tr);
                } else {
                    (*vmi)->setSceneMatrix (this->sceneview);
                }
                (*vmi)->render();
                ++vmi;
            }

            sm::vec<float, 3> v0 = this->textPosition ({-0.8f, 0.8f});
            if (this->options.test (visual_options::showTitle) == true) {
                // Render the title text
                this->textModel->setSceneTranslation (v0);
                this->textModel->setVisibleOn (this->bgcolour);
                this->textModel->render();
            }

            auto ti = this->texts.begin();
            while (ti != this->texts.end()) {
                (*ti)->setSceneTranslation (v0);
                (*ti)->setVisibleOn (this->bgcolour);
                (*ti)->render();
                ++ti;
            }

            if (this->options.test (visual_options::renderSwapsBuffers) == true) {
                this->swapBuffers();
            }
        }

    public:
        void init_glad (GLADloadfunc procaddressfn)
        {
            this->glfn_version = gladLoadGL (procaddressfn);
            if (this->glfn_version == 0) {
                throw std::runtime_error ("Failed to initialize GLAD GL context");
            }
        }

        //! Add a label _text to the scene at position _toffset. Font features are
        //! defined by the tfeatures. Return geometry of the text.
        mplot::TextGeometry addLabel (const std::string& _text,
                                      const sm::vec<float, 3>& _toffset,
                                      const mplot::TextFeatures& tfeatures = mplot::TextFeatures(0.01f))
        {
            this->setContext();
            if (this->shaders.tprog == 0) { throw std::runtime_error ("No text shader prog."); }
            auto tmup = std::make_unique<mplot::VisualTextModel<glver>> (tfeatures);
            this->bindmodel (tmup);
            if (tfeatures.centre_horz == true) {
                mplot::TextGeometry tg = tmup->getTextGeometry(_text);
                sm::vec<float, 3> centred_locn = _toffset;
                centred_locn[0] = -tg.half_width();
                tmup->setupText (_text, centred_locn, tfeatures.colour);
            } else {
                tmup->setupText (_text, _toffset, tfeatures.colour);
            }
            mplot::VisualTextModel<glver>* tm = tmup.get();
            this->texts.push_back (std::move(tmup));
            this->releaseContext();
            return tm->getTextGeometry();
        }

        //! Add a label _text to the scene at position _toffset. Font features are
        //! defined by the tfeatures. Return geometry of the text. The pointer tm is a
        //! return value that allows client code to change the text after the label has been added.
        mplot::TextGeometry addLabel (const std::string& _text,
                                      const sm::vec<float, 3>& _toffset,
                                      mplot::VisualTextModel<glver>*& tm,
                                      const mplot::TextFeatures& tfeatures = mplot::TextFeatures(0.01f))
        {
            this->setContext();
            if (this->shaders.tprog == 0) { throw std::runtime_error ("No text shader prog."); }
            auto tmup = std::make_unique<mplot::VisualTextModel<glver>> (tfeatures);
            this->bindmodel (tmup);
            if (tfeatures.centre_horz == true) {
                mplot::TextGeometry tg = tmup->getTextGeometry(_text);
                sm::vec<float, 3> centred_locn = _toffset;
                centred_locn[0] = -tg.half_width();
                tmup->setupText (_text, centred_locn, tfeatures.colour);
            } else {
                tmup->setupText (_text, _toffset, tfeatures.colour);
            }
            tm = tmup.get();
            this->texts.push_back (std::move(tmup));
            this->releaseContext();
            return tm->getTextGeometry();
        }

    protected:
        // Initialize OpenGL shaders, set some flags (Alpha, Anti-aliasing), read in any external
        // state from json, and set up the coordinate arrows and any VisualTextModels that will be
        // required to render the Visual.
        void init_gl()
        {
            this->setContext(); // if managing context

            if (this->options.test (visual_options::versionStdout) == true) {
                unsigned char* glv = (unsigned char*)glGetString(GL_VERSION);
                std::cout << "This is version " << mplot::version_string()
                          << " of mplot::Visual<glver=" << mplot::gl::version::vstring (glver)
                          << "> running on OpenGL Version " << glv << std::endl;
            }

            this->setSwapInterval();

            // Load up the shaders
            this->proj2d_shader_progs = {
                {GL_VERTEX_SHADER, "Visual.vert.glsl", mplot::getDefaultVtxShader(glver), 0 },
                {GL_FRAGMENT_SHADER, "Visual.frag.glsl", mplot::getDefaultFragShader(glver), 0 }
            };
            this->shaders.gprog = mplot::gl::LoadShaders (this->proj2d_shader_progs);
            this->active_gprog = mplot::visgl::graphics_shader_type::projection2d;

            // Alternative cylindrical shader for possible later use. (NB: not loaded immediately)
            this->cyl_shader_progs = {
                {GL_VERTEX_SHADER, "VisCyl.vert.glsl", mplot::getDefaultCylVtxShader(glver), 0 },
                {GL_FRAGMENT_SHADER, "Visual.frag.glsl", mplot::getDefaultFragShader(glver), 0 }
            };

            // A specific text shader is loaded for text rendering
            this->text_shader_progs = {
                {GL_VERTEX_SHADER, "VisText.vert.glsl", mplot::getDefaultTextVtxShader(glver), 0 },
                {GL_FRAGMENT_SHADER, "VisText.frag.glsl" , mplot::getDefaultTextFragShader(glver), 0 }
            };
            this->shaders.tprog = mplot::gl::LoadShaders (this->text_shader_progs);

            // OpenGL options
            glEnable (GL_DEPTH_TEST);
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable (GL_CULL_FACE);
            mplot::gl::Util::checkError (__FILE__, __LINE__);

            this->read_scenetrans_from_json();

            // Use coordArrowsOffset to set the location of the CoordArrows *scene*
            this->coordArrows = std::make_unique<mplot::CoordArrows<glver>>();
            // For CoordArrows, because we don't add via Visual::addVisualModel(), we
            // have to set the get_shaderprogs function here:
            this->bindmodel (this->coordArrows);
            // And NOW we can proceed to init (lengths, thickness, em size for labels):
            this->coordArrows->init (sm::vec<>{0.1f, 0.1f, 0.1f}, 1.0f, 0.01f);
            this->coordArrows->finalize(); // VisualModel::finalize releases context (normally this is the right thing)...
            this->setContext();            // ...but we've got more work to do, so re-acquire context (if we're managing it)

            this->userFrame = std::make_unique<mplot::RodVisual<glver>>();
            this->bindmodel (this->userFrame);
            this->userFrame->init (sm::vec<float, 3>{},
                                   sm::vec<float, 3>{0.0f, 0.0f, -10.0f}, sm::vec<float, 3>{0.1f, 0.1f, 1.0f}, 0.05f,
                                   mplot::colour::turquoise2, mplot::colour::turquoise4);
            this->userFrame->face_uy = sm::vec<>::ux();
            this->userFrame->face_uz = sm::vec<>::uy();
            this->userFrame->finalize();
            this->setContext(); // see createCoordArrows() for comments

            mplot::gl::Util::checkError (__FILE__, __LINE__);

            // Set up the title, which may or may not be rendered
            mplot::TextFeatures title_tf(0.035f, 64);
            this->textModel = std::make_unique<mplot::VisualTextModel<glver>> (title_tf);
            this->bindmodel (this->textModel);
            this->textModel->setSceneTranslation ({0.0f, 0.0f, 0.0f});
            this->textModel->setupText (this->title);

            this->releaseContext();
        }

        //! A VisualTextModel for a title text.
        std::unique_ptr<mplot::VisualTextModel<glver>> textModel = nullptr;
        //! Text models for labels
        std::vector<std::unique_ptr<mplot::VisualTextModel<glver>>> texts;
    };

} // namespace mplot
