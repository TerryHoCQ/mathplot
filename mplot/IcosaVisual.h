#pragma once

#include <array>
#include <sm/mathconst>
#include <sm/vec>
#include <mplot/VisualModel.h>

namespace mplot {

    //! This class creates the vertices for an icosahedron in a 3D scene.
    template<int glver = mplot::gl::version_4_1>
    class IcosaVisual : public VisualModel<glver>
    {
    public:
        IcosaVisual() {}

        //! Initialise with offset, start and end coordinates, radius and a single colour.
        IcosaVisual(const sm::vec<float, 3> _offset,
                    const float _radius,
                    const std::array<float, 3> _col)
        {
            this->init (_offset, _radius, _col);
        }

        ~IcosaVisual () {}

        void init (const sm::vec<float, 3> _offset,
                   const float _radius,
                   const std::array<float, 3> _col)
        {
            this->viewmatrix.translate (_offset);
            this->radius = _radius;
            for (auto& c : this->colours) { c = _col; }
        }

        static constexpr bool use_oriented_tube = false;
        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();

            this->computeIcosahedron (sm::vec<float, 3>({0,0,0}), this->colours, this->radius);
        }

        //! The radius of the icosahedrona
        float radius = 1.0f;
        //! The colours of the object
        std::array<std::array<float, 3>, 20> colours;
    };

} // namespace mplot
