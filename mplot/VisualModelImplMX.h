/*!
 * \file
 *
 * Declares a VisualModel implementation class, adding multi-context-safe GLAD GL function calls.
 *
 * \author Seb James
 * \date March 2025
 */

#pragma once

#if defined __gl3_h_ || defined __gl_h_
// GL headers have been externally included
#else
# error "GL headers should have been included already"
#endif

#include <type_traits>

#include <mplot/VisualModelBase.h>

#include <mplot/gl/util_mx.h>
#include <mplot/VisualTextModel.h>
#include <mplot/TextGeometry.h>

namespace mplot {

    //! Forward declaration of base classes
    template <int> class VisualBase;
    template <int> class VisualOwnableMX;

    /*!
     * Multiple context safe implementation (mplot::gl::multicontext == 1)
     */
    template <int glver = mplot::gl::version_4_1, int mx = mplot::gl::multicontext, std::enable_if_t<mx==1, bool> = true >
    struct VisualModelImpl : public mplot::VisualModelBase<glver>
    {
        VisualModelImpl() : mplot::VisualModelBase<glver>::VisualModelBase() {}
        VisualModelImpl (const sm::vec<float>& _offset) : mplot::VisualModelBase<glver>::VisualModelBase(_offset) {}

        //! destroy gl buffers in the deconstructor
        virtual ~VisualModelImpl() // clang gives -Wdelete-non-abstract-non-virtual-dtor without virtual
        {
            // Explicitly clear owned VisualTextModels
            this->texts.clear();
            if (this->vbos != nullptr) {
                GladGLContext* _glfn = this->get_glfn(this->parentVis);
                _glfn->DeleteBuffers (this->numVBO, this->vbos.get());
                _glfn->DeleteVertexArrays (1, &this->vao);
            }
        }

        /*!
         * Set up the passed-in VisualTextModel with functions that need access to the parent Visual attributes.
         */
        template <typename T>
        void bindmodel (std::unique_ptr<T>& model)
        {
            if (this->parentVis == nullptr) {
                throw std::runtime_error ("Can't bind a model, because I am not bound");
            }
            model->set_parent (this->parentVis);
            model->get_shaderprogs = &mplot::VisualBase<glver>::get_shaderprogs;
            model->get_gprog = &mplot::VisualBase<glver>::get_gprog;
            model->get_tprog = &mplot::VisualBase<glver>::get_tprog;

            model->get_glfn = &mplot::VisualOwnableMX<glver>::get_glfn;

            model->setContext = &mplot::VisualBase<glver>::set_context;
            model->releaseContext = &mplot::VisualBase<glver>::release_context;
        }

        //! Common code to call after the vertices have been set up. GL has to have been initialised.
        void postVertexInit() final
        {
            GladGLContext* _glfn = this->get_glfn (this->parentVis);

            // Do gl memory allocation of vertex array once only
            if (this->vbos == nullptr) {
                // Create vertex array object
                _glfn->GenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
            }
            _glfn->BindVertexArray (this->vao);

            // Create the vertex buffer objects (once only)
            if (this->vbos == nullptr) {
                this->vbos = std::make_unique<GLuint[]>(this->numVBO);
                _glfn->GenBuffers (this->numVBO, this->vbos.get()); // OpenGL 4.4- safe
            }

            // Set up the indices buffer - bind and buffer the data in this->indices
            _glfn->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->vbos[this->idxVBO]);

            std::size_t sz = this->indices.size() * sizeof(GLuint);
            _glfn->BufferData (GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[this->posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[this->normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[this->colVBO], this->vertexColors, visgl::colLoc);

            // Unbind only the vertex array (not the buffers, that causes GL_INVALID_ENUM errors)
            _glfn->BindVertexArray(0); // carefully unbind and rebind
            mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);

            /*
             * Now do the same for the bounding box
             */
            if (this->flags.test (vm_bools::compute_bb)) {

                if (this->vbos_bb == nullptr) { _glfn->GenVertexArrays (1, &this->vao_bb); }
                _glfn->BindVertexArray (this->vao_bb);

                // Create the vertex buffer objects (once only)
                if (this->vbos_bb == nullptr) {
                    this->vbos_bb = std::make_unique<GLuint[]>(this->numVBO);
                    _glfn->GenBuffers (this->numVBO, this->vbos_bb.get());
                }

                // Set up the indices buffer - bind and buffer the data in this->indices
                _glfn->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->vbos_bb[this->idxVBO]);

                std::size_t sz = this->indices_bb.size() * sizeof(GLuint);
                _glfn->BufferData (GL_ELEMENT_ARRAY_BUFFER, sz, this->indices_bb.data(), GL_STATIC_DRAW);

                // Binds data from the "C++ world" to the OpenGL shader world for
                // "position", "normalin" and "color"
                // (bind, buffer and set vertex array object attribute)
                this->setupVBO (this->vbos_bb[this->posnVBO], this->vpos_bb, visgl::posnLoc);
                this->setupVBO (this->vbos_bb[this->normVBO], this->vnorm_bb, visgl::normLoc);
                this->setupVBO (this->vbos_bb[this->colVBO], this->vcol_bb, visgl::colLoc);

                // Unbind only the vertex array (not the buffers, that causes GL_INVALID_ENUM errors)
                _glfn->BindVertexArray(0); // carefully unbind and rebind
                mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);
            }

            this->flags.set (vm_bools::postVertexInitRequired, false);
        }

        //! Initialize vertex buffer objects and vertex array object. Empty for 'text only' VisualModels.
        void initializeVertices() {};

        /*!
         * Re-initialize the buffers. Client code might have appended to
         * vertexPositions/Colors/Normals and indices before calling this method.
         */
        void reinit_buffers() final
        {
            GladGLContext* _glfn = this->get_glfn(this->parentVis);
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            if (this->flags.test (vm_bools::postVertexInitRequired) == true) { this->postVertexInit(); }

            // Now re-set up the VBOs
            _glfn->BindVertexArray (this->vao);                                    // carefully unbind and rebind
            _glfn->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->vbos[this->idxVBO]);  // carefully unbind and rebind

            std::size_t sz = this->indices.size() * sizeof(GLuint);
            _glfn->BufferData (GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            this->setupVBO (this->vbos[this->posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[this->normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[this->colVBO], this->vertexColors, visgl::colLoc);

            _glfn->BindVertexArray(0);                                // carefully unbind and rebind
            mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);  // carefully unbind and rebind

            // Optional bounding box
            if (this->flags.test (vm_bools::compute_bb)) {
                _glfn->BindVertexArray (this->vao_bb);
                _glfn->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->vbos_bb[this->idxVBO]);

                std::size_t sz = this->indices_bb.size() * sizeof(GLuint);
                _glfn->BufferData (GL_ELEMENT_ARRAY_BUFFER, sz, this->indices_bb.data(), GL_STATIC_DRAW);
                this->setupVBO (this->vbos_bb[this->posnVBO], this->vpos_bb, visgl::posnLoc);
                this->setupVBO (this->vbos_bb[this->normVBO], this->vnorm_bb, visgl::normLoc);
                this->setupVBO (this->vbos_bb[this->colVBO], this->vcol_bb, visgl::colLoc);

                _glfn->BindVertexArray(0);
                mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);
            }
        }

        //! reinit ONLY vertexColors buffer
        void reinit_colour_buffer() final
        {
            if (this->setContext != nullptr) { this->setContext (this->parentVis); }
            if (this->flags.test (vm_bools::postVertexInitRequired) == true) { this->postVertexInit(); }
            GladGLContext* _glfn = this->get_glfn(this->parentVis);
            // Now re-set up the VBOs
            _glfn->BindVertexArray (this->vao);  // carefully unbind and rebind
            this->setupVBO (this->vbos[this->colVBO], this->vertexColors, visgl::colLoc);
            _glfn->BindVertexArray(0);  // carefully unbind and rebind
            mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);
        }

        void clearTexts() { this->texts.clear(); }

        static constexpr bool debug_render = false;
        //! Render the VisualModel. Note that it is assumed that the OpenGL context has been
        //! obtained by the parent Visual::render call.
        void render() // not final
        {
            if (this->hidden() == true) { return; }

            // Execute post-vertex init at render, as GL should be available.
            if (this->flags.test (vm_bools::postVertexInitRequired) == true) { this->postVertexInit(); }

            GLint prev_shader = 0;

            GladGLContext* _glfn = this->get_glfn (this->parentVis);
            _glfn->GetIntegerv (GL_CURRENT_PROGRAM, &prev_shader);
            // Ensure the correct program is in play for this VisualModel
            _glfn->UseProgram (this->get_gprog(this->parentVis));

            if (!this->indices.empty()) {
                // It is only necessary to bind the vertex array object before rendering
                // (not the vertex buffer objects)
                _glfn->BindVertexArray (this->vao);

                // Pass this->float to GLSL so the model can have an alpha value.
                GLint loc_a = _glfn->GetUniformLocation (this->get_gprog(this->parentVis), static_cast<const GLchar*>("alpha"));
                if (loc_a != -1) { _glfn->Uniform1f (loc_a, this->alpha); }

                // The scene-view matrix
                GLint loc_v = _glfn->GetUniformLocation (this->get_gprog(this->parentVis), static_cast<const GLchar*>("v_matrix"));
                if (loc_v != -1) { _glfn->UniformMatrix4fv (loc_v, 1, GL_FALSE, this->scenematrix.mat.data()); }

                // the model-view matrix
                GLint loc_m = _glfn->GetUniformLocation (this->get_gprog(this->parentVis), static_cast<const GLchar*>("m_matrix"));
                if (loc_m != -1) { _glfn->UniformMatrix4fv (loc_m, 1, GL_FALSE, this->viewmatrix.mat.data()); }

                if constexpr (debug_render) {
                    std::cout << "VisualModel::render: scenematrix:\n" << this->scenematrix << std::endl;
                    std::cout << "VisualModel::render: model viewmatrix:\n" << this->viewmatrix << std::endl;
                }

                // Draw the triangles
                _glfn->DrawElements (GL_TRIANGLES, static_cast<unsigned int>(this->indices.size()), GL_UNSIGNED_INT, 0);

                // Unbind the VAO
                _glfn->BindVertexArray(0);

                // Do the bounding box optionally
                if (this->flags.test (vm_bools::compute_bb) && this->flags.test (vm_bools::show_bb) && !this->indices_bb.empty()) {
                    _glfn->BindVertexArray (this->vao_bb);
                    _glfn->DrawElements (GL_TRIANGLES, static_cast<unsigned int>(this->indices_bb.size()), GL_UNSIGNED_INT, 0);
                    _glfn->BindVertexArray(0);
                }
            }
            mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);

            // Now render any VisualTextModels
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->render(); ti++; }

            _glfn->UseProgram (prev_shader);
            mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);
        }


        /*!
         * Helper to make a VisualTextModel and bind it ready for use.
         *
         * You could write it out explicitly as:
         *
         * std::unique_ptr<mplot::VisualTextModel<glver>> vtm1 = this->makeVisualTextModel (tfca);
         *
         * Or use auto to help:
         *
         * auto vtm1 = this->makeVisualTextModel (tfca);
         *
         * See GraphVisual.h for examples.
         */
        std::unique_ptr<mplot::VisualTextModel<glver>> makeVisualTextModel(const mplot::TextFeatures& tfeatures)
        {
            auto tmup = std::make_unique<mplot::VisualTextModel<glver>> (tfeatures);
            this->bindmodel (tmup);
            return tmup;
        }

        /*!
         * Add a text label to the model at location (within the model coordinates)
         * toffset. Return the text geometry of the added label so caller can place
         * associated text correctly.  Control font size, resolution, colour and font
         * face with tfeatures.
         */
        mplot::TextGeometry addLabel (const std::string& _text,
                                      const sm::vec<float, 3>& _toffset,
                                      const mplot::TextFeatures& tfeatures = mplot::TextFeatures())
        {
            if (this->get_shaderprogs(this->parentVis).tprog == 0) {
                throw std::runtime_error ("No text shader prog. Did your VisualModel-derived class set it up?");
            }

            if (this->setContext != nullptr) { this->setContext (this->parentVis); } // For VisualTextModel

            auto tmup = this->makeVisualTextModel (tfeatures);

            if (tfeatures.centre_horz == true) {
                mplot::TextGeometry tg = tmup->getTextGeometry(_text);
                sm::vec<float, 3> centred_locn = _toffset;
                centred_locn[0] -= tg.half_width();
                tmup->setupText (_text, centred_locn + this->viewmatrix.translation(), tfeatures.colour);
            } else {
                tmup->setupText (_text, _toffset + this->viewmatrix.translation(), tfeatures.colour);
            }

            this->texts.push_back (std::move(tmup));

            // As this is a setup function, release the context
            if (this->releaseContext != nullptr) { this->releaseContext (this->parentVis); }

            return this->texts.back()->getTextGeometry();
        }

        /*!
         * Add a text label, with given offset _toffset and the specified tfeatures. The
         * reference to a pointer, tm, allows client code to change the text of the
         * VisualTextModel as necessary, after the label has been added.
         */
        mplot::TextGeometry addLabel (const std::string& _text,
                                      const sm::vec<float, 3>& _toffset,
                                      mplot::VisualTextModel<glver>*& tm,
                                      const mplot::TextFeatures& tfeatures = mplot::TextFeatures())
        {
            if (this->get_shaderprogs(this->parentVis).tprog == 0) {
                throw std::runtime_error ("No text shader prog. Did your VisualModel-derived class set it up?");
            }

            if (this->setContext != nullptr) { this->setContext (this->parentVis); } // For VisualTextModel

            auto tmup = this->makeVisualTextModel (tfeatures);

            if (tfeatures.centre_horz == true) {
                mplot::TextGeometry tg = tmup->getTextGeometry(_text);
                sm::vec<float, 3> centred_locn = _toffset;
                centred_locn[0] -= tg.half_width();
                tmup->setupText (_text, centred_locn + this->viewmatrix.translation(), tfeatures.colour);
            } else {
                tmup->setupText (_text, _toffset + this->viewmatrix.translation(), tfeatures.colour);
            }

            this->texts.push_back (std::move(tmup));
            tm = this->texts.back().get();

            // As this is a setup function, release the context
            if (this->releaseContext != nullptr) { this->releaseContext (this->parentVis); }

            return this->texts.back()->getTextGeometry();
        }

        void setSceneMatrixTexts (const sm::mat44<float>& sv) final
        {
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->setSceneMatrix (sv); ti++; }
        }

        void setSceneTranslationTexts (const sm::vec<float>& v0) final
        {
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->setSceneTranslation (v0); ti++; }
        }

        void setViewRotationTexts (const sm::quaternion<float>& r)
        {
            // When rotating a model that contains texts, we need to rotate the scene
            // for the texts and also inverse-rotate the view of the texts.
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) {
                // Rotate the scene. Note this won't work if the VisualModel has a
                // translation away from the origin.
                (*ti)->setSceneRotation (r); // Need this to rotate about _offset. BUT the
                                             // translation is already there in the text,
                                             // but in the MODEL view.

                // Rotate the view of the text an opposite amount, to keep it facing forwards
                (*ti)->setViewRotation (r.invert());
                ti++;
            }
        }

        void addViewRotationTexts (const sm::quaternion<float>& r)
        {
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->addViewRotation (r); ti++; }
        }

        //! Get the GladGLContext function pointer
        std::function<GladGLContext*(mplot::VisualBase<glver>*)> get_glfn;

    protected:

        //! A vector of pointers to text models that should be rendered.
        std::vector<std::unique_ptr<mplot::VisualTextModel<glver>>> texts;

        //! Set up a vertex buffer object - bind, buffer and set vertex array object attribute
        void setupVBO (GLuint& buf, std::vector<float>& dat, unsigned int bufferAttribPosition) final
        {
            std::size_t sz = dat.size() * sizeof(float);

            GladGLContext* _glfn = this->get_glfn(this->parentVis);
            _glfn->BindBuffer (GL_ARRAY_BUFFER, buf);
            mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);
            _glfn->BufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);
            _glfn->VertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);
            _glfn->EnableVertexAttribArray (bufferAttribPosition);
            mplot::gl::Util::checkError (__FILE__, __LINE__, _glfn);
        }
    };

} // namespace mplot
