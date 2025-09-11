#pragma once

#include <array>
#include <sm/vec>
#include <mplot/VisualModel.h>

namespace mplot {

    //! This class creates the vertices for a polygonal object in a 3D scene
    template<int glver = mplot::gl::version_4_1>
    class PolygonVisual : public VisualModel<glver>
    {
    public:
        PolygonVisual() {}

        PolygonVisual(const sm::vec<float, 3> _offset,
                      const sm::vec<float, 3> _position, const sm::vec<float, 3> _vertex,
                      const float _radius, const float _thickness,
                      const std::array<float, 3> _col, const int _n)
        {
            this->init (_offset, _position, _vertex, _radius, _thickness, _col, _n);
        }

        void init (const sm::vec<float, 3> _offset,
                   const sm::vec<float, 3> _position, const sm::vec<float, 3> _vertex,
                   const float _radius, const float _thickness,
                   const std::array<float, 3> _col, const int _n)
        {
            this->viewmatrix.translate (_offset);
            this->position = _position;
            this->vertex = _vertex;
            this->radius = _radius;
            this->thickness = _thickness;
            this->col = _col;
            this->n = _n;
        }

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();
            this->idx = 0;

            // Always draw a full 3D polygon
            sm::vec<float> pend = this->position;
            pend[2] += this->thickness;
            // Figure out ux, uy from position and vertex. Let ux be like dirn to vertex
            this->_ux = this->vertex - this->position;
            this->_uy = this->_ux.cross(sm::vec<>::uz());
            this->computeTube (this->position, pend, this->_ux, this->_uy,
                               this->col, this->col,
                               this->radius, this->n);
        }

        //! The position of the start of the rod, given with respect to the parent's offset
        sm::vec<float, 3> position = {0.0f, 0.0f, 0.0f};
        //! Direction to the first vertex.
        sm::vec<float, 3> vertex = {1.0f, 0.0f, 0.0f};
        //! The radius of the polygonal puck's enclosing circle
        float radius = 1.0f;
        //! The thickness of the polygonal puck
        float thickness = 0.01f;
        //! Number of segments in this polygon
        int n = 4;

        // Some axes
        sm::vec<float> _ux = sm::vec<>::ux();
        sm::vec<float> _uy = sm::vec<>::uy();

        //! The colour of the thing.
        std::array<float, 3> col = {1.0f, 0.0f, 0.0f};
    };

} // namespace mplot
