/*
 * Visualize a Sphere
 */
#include <memory>
#include <iostream>
#include <stdexcept>
#include <cmath>

import mplot.visual;
import mplot.visualmodel;

// Quick visual that simply draws spheres
template <int glver = mplot::gl::version_4_1>
class PrimitiveVisual : public mplot::VisualModel<glver>
{
public:
    PrimitiveVisual (const sm::vec<float> _offset) { this->viewmatrix.translate (_offset); }

    void initializeVertices()
    {
        // This primitive computes a fan and rings to make a sphere
        float l = 1.1f;
        this->computeSphere (sm::vec<float>{-l,0,0}, mplot::colour::royalblue, 1.0f, 12, 12);
        // These compute the sphere from a geodesic icosahedron. First with 2 iterations of the triangulation algorithm
        this->computeSphereGeo (sm::vec<float>{l,0,0}, mplot::colour::maroon, 1.0f, 2);
        // This one with 3 iterations (meaning more triangles and a smoother sphere) and compile time geodesic computation
        this->template computeSphereGeoFast<float, 3> (sm::vec<float>{0,l * std::tan(60 * sm::mathconst<float>::deg2rad),0}, mplot::colour::cyan3, 1.0f);
    }
};

int main()
{
    mplot::Visual v(1024, 768, "Sphere primitives");
    v.lightingEffects (true);

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    auto pvm = std::make_unique<PrimitiveVisual<>> (offset);
    pvm->set_parent (v.get_id());
    pvm->finalize();
    v.addVisualModel (pvm);

    v.keepOpen();
}
