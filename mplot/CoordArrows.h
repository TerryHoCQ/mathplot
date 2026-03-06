/*!
 * \file
 *
 * Defines a non-VisualModel coordinate arrow class for internal use in the scene (bottom left corner).
 *
 * \author Seb James
 * \date 2019
 */
module;

#if defined __gl3_h_ || defined __gl_h_ // could get a fuller list from glfw.h
// GL headers appear to have been externally included.
#else
# include <mplot/glad/gl.h>
#endif // GL headers

#include <array>
#include <memory>
#include <vector>
#include <cmath>

#include <sm/mathconst>

#include <mplot/gl/version.h>
#include <mplot/gl/util_mx.h>
#include <mplot/colour.h>

export module mplot.core:coordarrows;

import mplot.visualtextmodel;
import mplot.textfeatures;
import mplot.visualfont;
import sm.vec;
import sm.mat;
import sm.flags;

export namespace mplot
{
    // State/options flags
    enum class ca_bools : uint32_t { postVertexInitRequired, hide };

    //! This class creates the vertices for a set of coordinate arrows to be rendered
    //! in a 3-D scene.
    template<int glver = mplot::gl::version_4_1>
    class CoordArrows
    {
    public:
        CoordArrows() {}
        CoordArrows (const sm::vec<float, 3>& offset) { this->viewmatrix.translate (offset); }

        //! Must make the boilerplate bindmodel call before calling init() (for text handling)
        void init (const sm::vec<float, 3> _lengths, const float _thickness, const float _em)
        {
            this->lengths = _lengths;
            this->thickness = _thickness;
            this->em = _em;
        }

        //! You can call this AS well as the first init overload to set the axis vectors
        void init (const sm::vec<float, 3> _x, const sm::vec<float, 3> _y, const sm::vec<float, 3> _z)
        {
            this->x_axis = _x;
            this->y_axis = _y;
            this->z_axis = _z;
        }

        // State/options flags
        constexpr sm::flags<ca_bools> flags_defaults()
        {
            sm::flags<ca_bools> _flags;
            _flags.reset(); // all false
            return _flags;
        }
        sm::flags<ca_bools> flags = flags_defaults();

        // The hide attribute accessors
        void setHide (const bool _h = true) { this->flags.set (ca_bools::hide, _h); }
        void toggleHide() { this->flags.flip (ca_bools::hide); }
        float hidden() const { return this->flags.test (ca_bools::hide); }

        void setSceneMatrixTexts (const sm::mat<float, 4>& sv)
        {
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->setSceneMatrix (sv); ti++; }
        }

        //! When setting the scene matrix, also have to set the text's scene matrices.
        void setSceneMatrix (const sm::mat<float, 4>& sv)
        {
            this->scenematrix = sv;
            this->setSceneMatrixTexts (sv);
        }

        void setSceneTranslationTexts (const sm::vec<float>& v0)
        {
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->setSceneTranslation (v0); ti++; }
        }

        //! Set a translation into the scene and into any child texts
        void setSceneTranslation (const sm::vec<float, 3>& v0)
        {
            this->scenematrix.set_identity();
            this->scenematrix.translate (v0);
            this->setSceneTranslationTexts (v0);
        }

        void setViewRotationTexts (const sm::quaternion<float>& r)
        {
            // See VisualModel for explanation
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) {
                (*ti)->setSceneRotation (r);
                (*ti)->setViewRotation (r.invert());
                ti++;
            }
        }
        //! Set a rotation (only) into the view
        void setViewRotation (const sm::quaternion<float>& r)
        {
            sm::vec<> os = this->viewmatrix.translation();
            this->viewmatrix.set_identity();
            this->viewmatrix.translate (os);
            this->viewmatrix.rotate (r);
            this->setViewRotationTexts (r);
        }

        //! Make sure coord arrow colours are ok on the given background colour. Call this *after* finalize.
        void setColourForBackground (const std::array<float, 4>& bgcolour)
        {
            // For now, only worry about the centresphere:
            std::array<float, 3> cscol = { 1.0f - bgcolour[0], 1.0f - bgcolour[1], 1.0f - bgcolour[2] };
            if (cscol != this->centresphere_col) {
                this->centresphere_col = cscol;
                this->reinit();
                // Give the text labels a suitable, visible colour
                // Don't worry about context, assume its ok.
                auto ti = this->texts.begin();
                while (ti != this->texts.end()) {
                    (*ti)->setVisibleOn (bgcolour);
                    ti++;
                }
            }
        }

        std::unique_ptr<mplot::VisualTextModel<glver>> makeVisualTextModel(const mplot::TextFeatures& tfeatures)
        {
            auto tmup = std::make_unique<mplot::VisualTextModel<glver>> (tfeatures);
            tmup->set_parent (this->parentVis);
            return tmup;
        }

        void initAxisLabels()
        {
            if (this->em > 0.0f) {

                mplot::TextFeatures tfca(this->em, 48, false, mplot::colour::black, mplot::VisualFont::DVSansItalic);

                // These texts are black by default
                sm::vec<float> _offset = this->viewmatrix.translation();
                sm::vec<float> toffset = {};
                toffset = _offset + this->x_axis * this->lengths[0];
                toffset[0] += this->em;
                auto vtm1 = this->makeVisualTextModel (tfca);
                vtm1->setupText (this->x_label, toffset);
                this->texts.push_back (std::move(vtm1));
                toffset = _offset + this->y_axis * this->lengths[1];
                toffset[0] += this->em;
                auto vtm2 = this->makeVisualTextModel (tfca);
                vtm2->setupText (this->y_label, toffset);
                this->texts.push_back (std::move(vtm2));
                toffset = _offset + this->z_axis * this->lengths[2];
                toffset[0] += this->em;
                auto vtm3 = this->makeVisualTextModel (tfca);
                vtm3->setupText (this->z_label, toffset);
                this->texts.push_back (std::move(vtm3));
            }
        }

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();
            this->idx = 0;

            // Draw four spheres to make up the coord frame, with centre at 0,0,0
            sm::vec<float, 3> reloffset = {};
            static constexpr sm::vec<float, 3> zerocoord = { 0.0f, 0.0f, 0.0f };
            this->computeSphere (zerocoord, centresphere_col, this->thickness * this->lengths[0] / 20.0f);

            // x
            reloffset = this->x_axis * this->lengths[0];
            this->computeSphere (reloffset, x_axis_col, (this->thickness * this->lengths[0] / 40.0f) * endsphere_size);
            this->computeTube (zerocoord, reloffset, x_axis_col, x_axis_col, this->thickness * this->lengths[0] / 80.0f);
            if (showneg) {
                this->computeTube (zerocoord, -reloffset, x_axis_neg, x_axis_neg, this->thickness * this->lengths[0] / 80.0f);
            }

            // y
            reloffset = this->y_axis * this->lengths[1];
            this->computeSphere (reloffset, y_axis_col, (this->thickness * this->lengths[0] / 40.0f) * endsphere_size);
            this->computeTube (zerocoord, reloffset, y_axis_col, y_axis_col, this->thickness * this->lengths[0] / 80.0f);
            if (showneg) {
                this->computeTube (zerocoord, -reloffset, y_axis_neg, y_axis_neg, this->thickness * this->lengths[0] / 80.0f);
            }

            // z
            reloffset = this->z_axis * this->lengths[2];
            this->computeSphere (reloffset, z_axis_col, (this->thickness * this->lengths[0] / 40.0f) * endsphere_size);
            this->computeTube (zerocoord, reloffset, z_axis_col, z_axis_col, this->thickness * this->lengths[0] / 80.0f);
            if (showneg) {
                this->computeTube (zerocoord, -reloffset, z_axis_neg, z_axis_neg, this->thickness * this->lengths[0] / 80.0f);
            }

            this->initAxisLabels();
        }

        void finalize()
        {
            this->initializeVertices();
            this->flags.set (ca_bools::postVertexInitRequired, true);
        }

        uint32_t gprog = 0;

        void render() // not final
        {
            if (this->hidden() == true) { return; }

            // Execute post-vertex init at render, as GL should be available.
            if (this->flags.test (ca_bools::postVertexInitRequired) == true) { this->postVertexInit(); }

            GLint prev_shader = 0;

            this->glfn->GetIntegerv (GL_CURRENT_PROGRAM, &prev_shader);
            // Ensure the correct program is in play for this VisualModel
            this->glfn->UseProgram (gprog);

            if (!this->indices.empty()) {

                this->glfn->PolygonMode (GL_FRONT_AND_BACK, GL_FILL); // filled not wireframe

                // It is only necessary to bind the vertex array object before rendering
                // (not the vertex buffer objects)
                this->glfn->BindVertexArray (this->vao);

                GLint loc_a = this->glfn->GetUniformLocation (gprog, static_cast<const GLchar*>("alpha"));
                if (loc_a != -1) { this->glfn->Uniform1f (loc_a, 1.0f); }

                // The scene-view matrix
                GLint loc_v = this->glfn->GetUniformLocation (gprog, static_cast<const GLchar*>("v_matrix"));
                if (loc_v != -1) { this->glfn->UniformMatrix4fv (loc_v, 1, GL_FALSE, this->scenematrix.arr.data()); }

                // the model-view matrix
                GLint loc_m = this->glfn->GetUniformLocation (gprog, static_cast<const GLchar*>("m_matrix"));
                if (loc_m != -1) { this->glfn->UniformMatrix4fv (loc_m, 1, GL_FALSE, this->viewmatrix.arr.data()); }

                // the instance scaling matrix (applied to all instances)
                GLint loc_s = this->glfn->GetUniformLocation (gprog, static_cast<const GLchar*>("s_matrix"));
                constexpr auto idmat = sm::mat<float, 4>::identity();
                if (loc_s != -1) { this->glfn->UniformMatrix4fv (loc_s, 1, GL_FALSE, idmat.arr.data()); }

                // Draw the triangles
                //GLint loc_is = this->glfn->GetUniformLocation (gprog, static_cast<const GLchar*>("instance_start"));
                //GLint loc_ic = this->glfn->GetUniformLocation (gprog, static_cast<const GLchar*>("instance_count"));
                //if (loc_is != -1) { this->glfn->Uniform1i (loc_is, -1); }
                //if (loc_ic != -1) { this->glfn->Uniform1i (loc_ic, -1); }
                this->glfn->DrawElements (GL_TRIANGLES, static_cast<unsigned int>(this->indices.size()), GL_UNSIGNED_INT, 0);

                // Unbind the VAO
                this->glfn->BindVertexArray(0);
            }
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);

            // Now render any VisualTextModels
            auto ti = this->texts.begin();
            while (ti != this->texts.end()) { (*ti)->render(); ti++; }

            this->glfn->UseProgram (prev_shader);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
        }

        //! Length multipliers that can be applied to ux, uy and uz
        sm::vec<float, 3> lengths = { 1.0f, 1.0f, 1.0f };

        //! The axes for the coordinate arrows. A simple right handed coordinate system aligned with
        //! the 'real' world coordinate system by default.
        sm::vec<float, 3> x_axis = { 1.0f, 0.0f, 0.0f };
        sm::vec<float, 3> y_axis = { 0.0f, 1.0f, 0.0f };
        sm::vec<float, 3> z_axis = { 0.0f, 0.0f, 1.0f };

        //! A thickness scaling factor, to apply to the arrows.
        float thickness = 1.0f;
        //! a multiplier on the end spheres
        float endsphere_size = 1.0f;
        //! m size for text labels
        float em = 0.0f;

        //! The colours of the arrows, and of the centre sphere (where default of black is suitable
        //! for a white background)
        std::array<float, 3> centresphere_col = mplot::colour::black;
        std::array<float, 3> x_axis_col = mplot::colour::crimson;
        std::array<float, 3> y_axis_col = mplot::colour::springgreen2;
        std::array<float, 3> z_axis_col = mplot::colour::blue2;

        bool showneg = false;
        std::array<float, 3> x_axis_neg = mplot::colour::raspberry;
        std::array<float, 3> y_axis_neg = mplot::colour::darkseagreen3;
        std::array<float, 3> z_axis_neg = mplot::colour::steelblue3;

        std::string x_label = "X";
        std::string y_label = "Y";
        std::string z_label = "Z";

        GladGLContext* glfn = nullptr;

        // The mplot::Visual in which this model exists.
        uint32_t parentVis = std::numeric_limits<uint32_t>::max();

    protected:

        //! The current indices index
        uint32_t idx = 0u;

        //! The model-specific view matrix. Used to transform the pose of the model in the scene.
        sm::mat<float, 4> viewmatrix = {};

        /*!
         * The scene view matrix. Each VisualModel has a copy of the scenematrix. It's set in
         * Visual::render. Different VisualModels may have different scenematrices (for example, the
         * CoordArrows has a different scenematrix from other VisualModels, and models marked
         * 'twodimensional' also have a different scenematrix).
         */
        sm::mat<float, 4> scenematrix = {};

        //! Contains the positions within the vbo array of the different vertex buffer objects
        enum VBOPos { posnVBO, normVBO, colVBO, idxVBO, numVBO };

        /*
         * Compute positions and colours of vertices for the hexes and store in these:
         */

        //! The OpenGL Vertex Array Object
        uint32_t vao = 0;

        //! Vertex Buffer Objects stored in an array
        std::unique_ptr<uint32_t[]> vbos;

        //! CPU-side data for indices
        std::vector<uint32_t> indices = {};
        //! CPU-side data for vertex positions
        std::vector<float> vertexPositions = {};
        //! CPU-side data for vertex normals
        std::vector<float> vertexNormals = {};
        //! CPU-side data for vertex colours
        std::vector<float> vertexColors = {};

        //! Common code to call after the vertices have been set up. GL has to have been initialised.
        void postVertexInit()
        {
            // Do gl memory allocation of vertex array once only
            if (this->vbos == nullptr) {
                // Create vertex array object
                this->glfn->GenVertexArrays (1, &this->vao); // Safe for OpenGL 4.4-
            }
            this->glfn->BindVertexArray (this->vao);

            // Create the vertex buffer objects (once only)
            if (this->vbos == nullptr) {
                this->vbos = std::make_unique<uint32_t[]>(this->numVBO);
                this->glfn->GenBuffers (this->numVBO, this->vbos.get()); // OpenGL 4.4- safe
            }

            // Set up the indices buffer - bind and buffer the data in this->indices
            this->glfn->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->vbos[this->idxVBO]);

            std::size_t sz = this->indices.size() * sizeof(uint32_t);
            this->glfn->BufferData (GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);

            // Binds data from the "C++ world" to the OpenGL shader world for
            // "position", "normalin" and "color"
            // (bind, buffer and set vertex array object attribute)
            this->setupVBO (this->vbos[this->posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[this->normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[this->colVBO], this->vertexColors, visgl::colLoc);

            // Unbind only the vertex array (not the buffers, that causes GL_INVALID_ENUM errors)
            this->glfn->BindVertexArray(0); // carefully unbind and rebind
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);

            this->flags.set (ca_bools::postVertexInitRequired, false); // Maybe just a bool here
        }

        //! Re-create the model - called after updating data
        void reinit()
        {
            // Fixme: Better not to clear, then repeatedly pushback here:
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();

            // NB: Do NOT call clearTexts() here! We're only updating the model itself.
            this->idx = 0u;
            this->initializeVertices();
            this->reinit_buffers();
        }

        /*!
         * Re-initialize the buffers. Client code might have appended to
         * vertexPositions/Colors/Normals and indices before calling this method.
         */
        void reinit_buffers()
        {
            // Note that we do not worry about setting context here, we assume the parent Visual has the context
            if (this->flags.test (ca_bools::postVertexInitRequired) == true) { this->postVertexInit(); }

            // Now re-set up the VBOs
            this->glfn->BindVertexArray (this->vao);                                    // carefully unbind and rebind
            this->glfn->BindBuffer (GL_ELEMENT_ARRAY_BUFFER, this->vbos[this->idxVBO]);  // carefully unbind and rebind

            std::size_t sz = this->indices.size() * sizeof(uint32_t);
            this->glfn->BufferData (GL_ELEMENT_ARRAY_BUFFER, sz, this->indices.data(), GL_STATIC_DRAW);
            this->setupVBO (this->vbos[this->posnVBO], this->vertexPositions, visgl::posnLoc);
            this->setupVBO (this->vbos[this->normVBO], this->vertexNormals, visgl::normLoc);
            this->setupVBO (this->vbos[this->colVBO], this->vertexColors, visgl::colLoc);

            this->glfn->BindVertexArray(0);                                // carefully unbind and rebind
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);  // carefully unbind and rebind
        }

        //! A vector of pointers to text models that should be rendered.
        std::vector<std::unique_ptr<mplot::VisualTextModel<glver>>> texts;

        //! Set up a vertex buffer object - bind, buffer and set vertex array object attribute
        void setupVBO (uint32_t& buf, std::vector<float>& dat, unsigned int bufferAttribPosition)
        {
            std::size_t sz = dat.size() * sizeof(float);

            this->glfn->BindBuffer (GL_ARRAY_BUFFER, buf);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            this->glfn->BufferData (GL_ARRAY_BUFFER, sz, dat.data(), GL_STATIC_DRAW);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            this->glfn->VertexAttribPointer (bufferAttribPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
            this->glfn->EnableVertexAttribArray (bufferAttribPosition);
            mplot::gl::Util::checkError (__FILE__, __LINE__, this->glfn);
        }

        void vertex_push (const float& x, const float& y, const float& z, std::vector<float>& vp)
        { vp.emplace_back (x); vp.emplace_back (y); vp.emplace_back (z); }
        template<std::size_t N = 3> requires (N == 3 || N == 4)
        void vertex_push (const std::array<float, N>& arr, std::vector<float>& vp)
        { vp.emplace_back (arr[0]); vp.emplace_back (arr[1]); vp.emplace_back (arr[2]); }
        template<std::size_t N = 3> requires (N == 3 || N == 4)
        void vertex_push (const sm::vec<float, N>& vec, std::vector<float>& vp)
        { vp.emplace_back (vec[0]); vp.emplace_back (vec[1]); vp.emplace_back (vec[2]); }

        /*!
         * Sphere, 1 colour version.
         *
         * \param so The sphere offset. Where to place this sphere...
         * \param sc The sphere colour.
         * \param r Radius of the sphere
         * \param rings Number of rings used to render the sphere
         * \param segments Number of segments used to render the sphere
         */
        void computeSphere (sm::vec<float> so, std::array<float, 3> sc,
                            float r = 1.0f, int rings = 10, int segments = 12)
        {
            float rings0 = -sm::mathconst<float>::pi_over_2;
            float _z0  = std::sin(rings0);
            float z0  = r * _z0;
            float r0 =  std::cos(rings0);
            float rings1 = sm::mathconst<float>::pi * (-0.5f + 1.0f / rings);
            float _z1 = std::sin(rings1);
            float z1 = r * _z1;
            float r1 = std::cos(rings1);

            this->vertex_push (so[0]+0.0f, so[1]+0.0f, so[2]+z0, this->vertexPositions);
            this->vertex_push (0.0f, 0.0f, -1.0f, this->vertexNormals);
            this->vertex_push (sc, this->vertexColors);

            uint32_t capMiddle = this->idx++;
            uint32_t ringStartIdx = this->idx;
            uint32_t lastRingStartIdx = this->idx;

            bool firstseg = true;
            for (int j = 0; j < segments; j++) {
                float segment = sm::mathconst<float>::two_pi * static_cast<float>(j) / segments;
                float x = std::cos(segment);
                float y = std::sin(segment);

                float _x1 = x*r1;
                float x1 = _x1*r;
                float _y1 = y*r1;
                float y1 = _y1*r;

                this->vertex_push (so[0]+x1, so[1]+y1, so[2]+z1, this->vertexPositions);
                this->vertex_push (_x1, _y1, _z1, this->vertexNormals);
                this->vertex_push (sc, this->vertexColors);

                if (!firstseg) {
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (this->idx-1);
                    this->indices.push_back (this->idx++);
                } else {
                    this->idx++;
                    firstseg = false;
                }
            }
            this->indices.push_back (capMiddle);
            this->indices.push_back (this->idx-1);
            this->indices.push_back (capMiddle+1);

            for (int i = 2; i < rings; i++) {

                rings0 = sm::mathconst<float>::pi * (-0.5f + static_cast<float>(i) / rings);
                _z0  = std::sin(rings0);
                z0  = r * _z0;
                r0 =  std::cos(rings0);

                for (int j = 0; j < segments; j++) {

                    float segment = sm::mathconst<float>::two_pi * static_cast<float>(j) / segments;
                    float x = std::cos(segment);
                    float y = std::sin(segment);

                    float _x0 = x*r0;
                    float x0 = _x0*r;
                    float _y0 = y*r0;
                    float y0 = _y0*r;

                    this->vertex_push (so[0]+x0, so[1]+y0, so[2]+z0, this->vertexPositions);
                    this->vertex_push (_x0, _y0, _z0, this->vertexNormals);
                    this->vertex_push (sc, this->vertexColors);

                    if (j == segments - 1) {
                        this->indices.push_back (ringStartIdx++);
                        this->indices.push_back (this->idx);
                        this->indices.push_back (lastRingStartIdx);
                        this->indices.push_back (lastRingStartIdx);
                        this->indices.push_back (this->idx++);
                        this->indices.push_back (lastRingStartIdx+segments);
                    } else {
                        this->indices.push_back (ringStartIdx++);
                        this->indices.push_back (this->idx);
                        this->indices.push_back (ringStartIdx);
                        this->indices.push_back (ringStartIdx);
                        this->indices.push_back (this->idx++);
                        this->indices.push_back (this->idx);
                    }
                }
                lastRingStartIdx += segments;
            }

            rings0 = sm::mathconst<float>::pi_over_2;
            _z0  = std::sin(rings0);
            z0  = r * _z0;
            r0 =  std::cos(rings0);

            this->vertex_push (so[0]+0.0f, so[1]+0.0f, so[2]+z0, this->vertexPositions);
            this->vertex_push (0.0f, 0.0f, 1.0f, this->vertexNormals);
            this->vertex_push (sc, this->vertexColors);
            capMiddle = this->idx++;
            firstseg = true;

            ringStartIdx = lastRingStartIdx;
            for (int j = 0; j < segments; j++) {
                if (j != segments - 1) {
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (ringStartIdx++);
                    this->indices.push_back (ringStartIdx);
                } else {
                    this->indices.push_back (capMiddle);
                    this->indices.push_back (ringStartIdx);
                    this->indices.push_back (lastRingStartIdx);
                }
            }
        } // end of sphere calculation

        /*!
         * Create a tube from \a start to \a end, with radius \a r and a colour which
         * transitions from the colour \a colStart to \a colEnd.
         *
         * \param idx The index into the 'vertex array'
         * \param start The start of the tube
         * \param end The end of the tube
         * \param colStart The tube starting colour
         * \param colEnd The tube's ending colour
         * \param r Radius of the tube
         * \param segments Number of segments used to render the tube
         */
        void computeTube (sm::vec<float> start, sm::vec<float> end,
                          std::array<float, 3> colStart, std::array<float, 3> colEnd,
                          float r = 1.0f, int segments = 12)
        {
            this->computeFlaredTube (start, end, colStart, colEnd, r, r, segments);
        }

        void computeFlaredTube (sm::vec<float> start, sm::vec<float> end,
                                std::array<float, 3> colStart, std::array<float, 3> colEnd,
                                float r = 1.0f, int segments = 12, float flare = 0.0f)
        {
            sm::vec<float> v = end - start;
            float l = v.length();
            float r_add = l * std::tan (std::abs(flare)) * (flare > 0.0f ? 1.0f : -1.0f);
            float r_end = r + r_add;
            this->computeFlaredTube (start, end, colStart, colEnd, r, r_end, segments);
        }

        void computeFlaredTube (sm::vec<float> start, sm::vec<float> end,
                                std::array<float, 3> colStart, std::array<float, 3> colEnd,
                                float r = 1.0f, float r_end = 1.0f, int segments = 12)
        {
            sm::vec<float> vstart = start;
            sm::vec<float> vend = end;
            sm::vec<float> v = vend - vstart;
            v.renormalize();

            sm::vec<float> rand_vec;
            rand_vec.randomize();
            sm::vec<float> inplane = rand_vec.cross(v);
            inplane.renormalize();

            sm::vec<float> v_x_inplane = v.cross(inplane);

            this->vertex_push (vstart, this->vertexPositions);
            this->vertex_push (-v, this->vertexNormals);
            this->vertex_push (colStart, this->vertexColors);

            for (int j = 0; j < segments; j++) {
                float t = j * sm::mathconst<float>::two_pi / static_cast<float>(segments);
                sm::vec<float> c = inplane * std::sin(t) * r + v_x_inplane * std::cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                this->vertex_push (-v, this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            for (int j = 0; j < segments; j++) {
                float t = j * sm::mathconst<float>::two_pi / static_cast<float>(segments);
                sm::vec<float> c = inplane * std::sin(t) * r + v_x_inplane * std::cos(t) * r;
                this->vertex_push (vstart+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (colStart, this->vertexColors);
            }

            for (int j = 0; j < segments; j++) {
                float t = (float)j * sm::mathconst<float>::two_pi / static_cast<float>(segments);
                sm::vec<float> c = inplane * std::sin(t) * r_end + v_x_inplane * std::cos(t) * r_end;
                this->vertex_push (vend+c, this->vertexPositions);
                c.renormalize();
                this->vertex_push (c, this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            for (int j = 0; j < segments; j++) {
                float t = (float)j * sm::mathconst<float>::two_pi / static_cast<float>(segments);
                sm::vec<float> c = inplane * std::sin(t) * r_end + v_x_inplane * std::cos(t) * r_end;
                this->vertex_push (vend+c, this->vertexPositions);
                this->vertex_push (v, this->vertexNormals);
                this->vertex_push (colEnd, this->vertexColors);
            }

            this->vertex_push (vend, this->vertexPositions);
            this->vertex_push (v, this->vertexNormals);
            this->vertex_push (colEnd, this->vertexColors);

            int nverts = (segments * 4) + 2;

            uint32_t capMiddle = this->idx;
            uint32_t capStartIdx = this->idx + 1u;
            uint32_t endMiddle = this->idx + static_cast<uint32_t>(nverts) - 1u;
            uint32_t endStartIdx = capStartIdx + (3u * segments);

            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (capMiddle);
                this->indices.push_back (capStartIdx + j);
                this->indices.push_back (capStartIdx + 1 + j);
            }

            this->indices.push_back (capMiddle);
            this->indices.push_back (capStartIdx + segments - 1);
            this->indices.push_back (capStartIdx);

            for (int lsection = 0; lsection < 3; ++lsection) {
                capStartIdx = this->idx + 1 + lsection*segments;
                endStartIdx = capStartIdx + segments;
                for (int j = 0; j < segments; j++) {
                    this->indices.push_back (capStartIdx + j);
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                    } else {
                        this->indices.push_back (capStartIdx + 1 + j);
                    }
                    this->indices.push_back (endStartIdx + j);
                    this->indices.push_back (endStartIdx + j);
                    if (j == (segments-1)) {
                        this->indices.push_back (endStartIdx);
                    } else {
                        this->indices.push_back (endStartIdx + 1 + j);
                    }
                    if (j == (segments-1)) {
                        this->indices.push_back (capStartIdx);
                    } else {
                        this->indices.push_back (capStartIdx + j + 1);
                    }
                }
            }

            for (int j = 0; j < segments-1; j++) {
                this->indices.push_back (endMiddle);
                this->indices.push_back (endStartIdx + j);
                this->indices.push_back (endStartIdx + 1 + j);
            }
            this->indices.push_back (endMiddle);
            this->indices.push_back (endStartIdx + segments - 1);
            this->indices.push_back (endStartIdx);

            this->idx += nverts;
        } // end computeFlaredTube with randomly initialized end vertices
    };

} // namespace mplot
