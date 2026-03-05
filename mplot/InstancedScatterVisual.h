/*!
 * An example of a scatter plot using instanced rendering
 *
 * \author Seb James
 * \date 2025
 */
#pragma once

#include <iostream>
#include <vector>
#include <array>
import sm.vec;
#include <mplot/tools.h>
#include <mplot/VisualModel.h>
#include <mplot/graphstyles.h>

namespace mplot
{
    template <int glver = mplot::gl::version_4_3>
    class InstancedScatterVisual : public VisualModel<glver>
    {
    public:
        InstancedScatterVisual (const sm::vec<float> _offset)
        {
            this->instanced (true);
            this->viewmatrix.translate (_offset);
        }

        void marker (const sm::vec<float> coord, const std::array<float, 3>& clr, const float size)
        {
            if (this->markers == mplot::markerstyle::rod) {
                // Draw a rod. markerdirn gives length and dirn. Radius from size
                sm::vec<float> hr = this->markerdirn * 0.5f; // half rod
                sm::vec<float> rs = coord + hr;
                sm::vec<float> re = coord - hr;
                this->computeTube (rs, re, clr, clr, size, 12);
            } else {
                if constexpr (draw_spheres_as_geodesics) {
                    // Slower than regular computeSphere(). 2 iterations gives 320 faces
                    this->template computeSphereGeoFast<float, 2> (coord, clr, size);
                } else {
                    // (16+2) * 20 gives 360 faces
                    this->computeSphere (coord, clr, size, 16, 20);
                }
            }
        }

        //! Compute spheres for a scatter plot
        void initializeVertices()
        {
            // Draw one marker. It will then be instanced as many using instanced rendering
            this->marker (sm::vec<>{}, mplot::colour::gold1, this->radiusFixed);
        }

        // The constexpr, unordered geodesic code is no slower than the regular
        // VisualModel::computeSphere(), but leave this off for now (if true, C++-20 is
        // required)
        static constexpr bool draw_spheres_as_geodesics = false;

        //! Set this->radiusFixed, then re-compute vertices.
        void setRadius (float fr)
        {
            this->radiusFixed = fr;
            this->reinit();
        }

        // How to show the scatter points?
        markerstyle markers = mplot::markerstyle::sphere;

        // Marker direction, if relevant. Used for length of rod markers
        sm::vec<float, 3> markerdirn = sm::vec<>::uz();

        //! Change this to get larger or smaller spheres.
        float radiusFixed = 0.05f;
    };

} // namespace mplot
