/*
 * You just want a sphere visual model? Here it is. This uses the beautiful icosahedral geodesic
 * sphere primitive (computeSphereGeoFast)
 */
#pragma once

#include <array>
#include <sm/vec>
#include <mplot/VisualModel.h>

namespace mplot
{
    //! This class creates the vertices for a simple sphere in a 3D scene.
    template<int iterations = 3, int glver = mplot::gl::version_4_1>
    class SphereVisual : public VisualModel<glver>
    {
    public:
        SphereVisual() {}

        //! Initialise with offset, radius and a single colour.
        SphereVisual(const sm::vec<float, 3> _offset, const float _radius, const std::array<float, 3> _col)
        {
            this->init (_offset, _radius, _col);
        }

        ~SphereVisual () {}

        void init (const sm::vec<float, 3> _offset, const float _radius, const std::array<float, 3> _col)
        {
            this->viewmatrix.translate (_offset);
            this->radius = _radius;
            this->sphere_colour = _col;
        }

        void initializeVertices()
        {
            this->template computeSphereGeoFast<float, iterations> (sm::vec<float>{}, this->sphere_colour, this->radius);
        }

        //! The radius of the sphere
        float radius = 1.0f;
        //! The colour of the sphere
        std::array<float, 3> sphere_colour = {1.0f, 0.0f, 0.0f};
    };

} // namespace mplot
