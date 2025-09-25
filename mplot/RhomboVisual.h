#pragma once

#include <array>
#include <sm/vec>
#include <mplot/VisualModel.h>
#include <mplot/ColourMap.h>

namespace mplot {

    //! This class creates the vertices for a rhombohedron
    template<int glver = mplot::gl::version_4_1>
    class RhomboVisual : public VisualModel<glver>
    {
    public:
        //! Initialise with offset, three edges and a single colour.
        RhomboVisual(const sm::vec<float, 3> _offset,
                     const sm::vec<float, 3> _edge1, const sm::vec<float, 3> _edge2, const sm::vec<float, 3> _edge3,
                     const std::array<float, 3> _col)
            : VisualModel<glver>(_offset)
            , edge1 (_edge1) , edge2 (_edge2) , edge3 (_edge3), col (_col) {}

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices() override
        {
            if (this->facecm == mplot::ColourMapType::fixed) {
                this->vertices_singlecolour();
            } else {
                this->vertices_multicolour();
            }
        }

        void vertices_singlecolour()
        {
            // Compute the face normals
            sm::vec<float> _n1 = this->edge1.cross (this->edge2);
            _n1.renormalize();
            sm::vec<float> _n2 = this->edge2.cross (this->edge3);
            _n2.renormalize();
            sm::vec<float> _n3 = this->edge1.cross (this->edge3);
            _n3.renormalize();

            // First corner of rhombohedron is at model-frame's origin
            sm::vec<float> o = {0,0,0};

            // Push positions and normals for 24 vertices to make up the rhombohedron; 4 for each face.
            // Front face
            this->vertex_push (o,                              this->vertexPositions);
            this->vertex_push (o + this->edge1,                this->vertexPositions);
            this->vertex_push (o + this->edge3,                this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge3,  this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (_n3, this->vertexNormals); }
            // Top face
            this->vertex_push (o + this->edge3, this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1 + this->edge3, this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (_n1, this->vertexNormals); }
            // Back face
            this->vertex_push (o + this->edge2 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1 + this->edge3, this->vertexPositions);
            this->vertex_push (o + this->edge2,                             this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1,               this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (-_n3, this->vertexNormals); }
            // Bottom face
            this->vertex_push (o + this->edge2,                this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1,  this->vertexPositions);
            this->vertex_push (o,                              this->vertexPositions);
            this->vertex_push (o + this->edge1,                this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (-_n1, this->vertexNormals); }
            // Left face
            this->vertex_push (o + this->edge2,                this->vertexPositions);
            this->vertex_push (o,                              this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge3,  this->vertexPositions);
            this->vertex_push (o + this->edge3,                this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (-_n2, this->vertexNormals); }
            // Right face
            this->vertex_push (o + this->edge1,                             this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge2,               this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge2 + this->edge3, this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (_n2, this->vertexNormals); }

            // Vertex colours are all the same
            for (unsigned short i = 0U; i < 24U; ++i) {
                this->vertex_push (this->col, this->vertexColors);
            }

            // Indices for 6 faces
            for (unsigned short i = 0U; i < 6U; ++i) {
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx--);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
            }
        }

        // Allowing a colour for each face. May be useful for debugging
        void vertices_multicolour()
        {
            // Compute the face normals
            sm::vec<float> _n1 = this->edge1.cross (this->edge2);
            _n1.renormalize();
            sm::vec<float> _n2 = this->edge2.cross (this->edge3);
            _n2.renormalize();
            sm::vec<float> _n3 = this->edge1.cross (this->edge3);
            _n3.renormalize();

            // First corner of rhombohedron is at model-frame's origin
            sm::vec<float> o = {0,0,0};

            // Push positions and normals for 24 vertices to make up the rhombohedron; 4 for each face.
            // Front face needs 6 vertices
            this->vertex_push (o,                              this->vertexPositions);
            this->vertex_push (o + this->edge1,                this->vertexPositions);
            this->vertex_push (o + this->edge3,                this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge3,  this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (_n3, this->vertexNormals); }
            // Top face
            this->vertex_push (o + this->edge3, this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1 + this->edge3, this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (_n1, this->vertexNormals); }
            // Back face
            this->vertex_push (o + this->edge2 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1 + this->edge3, this->vertexPositions);
            this->vertex_push (o + this->edge2,                             this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1,               this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (-_n3, this->vertexNormals); }
            // Bottom face
            this->vertex_push (o + this->edge2,                this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge1,  this->vertexPositions);
            this->vertex_push (o,                              this->vertexPositions);
            this->vertex_push (o + this->edge1,                this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (-_n1, this->vertexNormals); }
            // Left face
            this->vertex_push (o + this->edge2,                this->vertexPositions);
            this->vertex_push (o,                              this->vertexPositions);
            this->vertex_push (o + this->edge2 + this->edge3,  this->vertexPositions);
            this->vertex_push (o + this->edge3,                this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (-_n2, this->vertexNormals); }
            // Right face
            this->vertex_push (o + this->edge1,                             this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge2,               this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge3,               this->vertexPositions);
            this->vertex_push (o + this->edge1 + this->edge2 + this->edge3, this->vertexPositions);
            for (unsigned short i = 0U; i < 4U; ++i) { this->vertex_push (_n2, this->vertexNormals); }

            // Vertex colours are all the same
            mplot::ColourMap<float> cm (this->facecm);
            for (unsigned short i = 0U; i < 6U; ++i) {
                this->vertex_push (cm.convert(), this->vertexColors);
            }

            // Indices for 6 faces
            for (unsigned short i = 0U; i < 6U; ++i) {
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx--);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
                this->indices.push_back (this->idx++);
            }
        }

        //! Three vectors define the Rhombohedron and we use a single colour
        sm::vec<float, 3> edge1 = {0.0f, 0.0f, 0.0f};
        sm::vec<float, 3> edge2 = {0.0f, 0.0f, 0.0f};
        sm::vec<float, 3> edge3 = {0.0f, 0.0f, 0.0f};
        std::array<float, 3> col = {0.0f, 0.0f, 1.0f};
        mplot::ColourMapType facecm = mplot::ColourMapType::Fixed; // if so, use col
    };

} // namespace mplot
