/*!
 * An example of a scatter plot using instanced rendering
 *
 * \author Seb James
 * \date 2025
 */
module;

#include <cstdint>
#include <iostream>
#include <vector>
#include <array>

export module mplot.instancedscattervisual;

export import sm.vec;
export import mplot.graphstyles;
export import mplot.colour;
export import mplot.visualmodel;
import mplot.tools;

export namespace mplot
{
    template <std::int32_t glver = mplot::gl::version_4_3>
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
                sm::vec<float> rs = coord + (marker_offset * marker_offset_dirn) + hr;
                sm::vec<float> re = coord + (marker_offset * marker_offset_dirn) - hr;
                this->computeTube (rs, re, clr, clr, size, 12);
            } else {
                if constexpr (draw_spheres_as_geodesics) {
                    // Slower than regular computeSphere(). 2 iterations gives 320 faces
                    this->template computeSphereGeoFast<float, 2> (coord + (marker_offset * marker_offset_dirn), clr, size);
                } else {
                    // (16+2) * 20 gives 360 faces
                    this->computeSphere (coord + (marker_offset * marker_offset_dirn), clr, size, 16, 20);
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

        // Marker direction, if relevant. Used for length of rod markers or offset of spheres
        sm::vec<float, 3> markerdirn = sm::vec<>::uz();

        // You may wish to offset your spheres so they sit on a surface. In which case, set
        // marker_offset with the same value as radiusFixed, and set marker_offset_dirn
        // appropriately (make sure it's a unit vector).
        sm::vec<float, 3> marker_offset_dirn = sm::vec<>::uz();
        float marker_offset = 0.0f;

        //! Change this to get larger or smaller spheres.
        float radiusFixed = 0.05f;
    };

} // namespace mplot
