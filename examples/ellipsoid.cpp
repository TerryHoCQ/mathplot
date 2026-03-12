/*
 * Visualize an ellipsoid
 */
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <memory>

import sm.mathconst;
import sm.mat;

import mplot.visual;
import mplot.normalsvisual;

// Quick visual that simply draws ellipsoid
template <int glver = mplot::gl::version_4_1>
class PrimitiveVisual : public mplot::VisualModel<glver>
{
public:
    PrimitiveVisual (const sm::vec<float> _offset) { this->viewmatrix.translate (_offset); }

    void initializeVertices()
    {
        sm::mat<float, 4> tr;
        tr.rotate (sm::vec<>::uz(), sm::mathconst<float>::pi_over_4);
        this->computeEllipsoid (sm::vec<float>{0},
                                mplot::colour::royalblue,
                                mplot::colour::maroon3,
                                sm::vec<float>{1,2,3},
                                40, 40, tr);
    }
};

int main()
{
    mplot::Visual v(1024, 768, "Ellipsoid primitive");
    v.lightingEffects (true);

    auto pvm = std::make_unique<PrimitiveVisual<>> (sm::vec<>{});
    pvm->set_parent (v.get_id());
    pvm->finalize();
    auto pvmp = v.addVisualModel (pvm);

    constexpr bool show_normals = true;
    if constexpr (show_normals) {
        // Create an associate normals model
        auto nrm = std::make_unique<mplot::NormalsVisual<>> (pvmp);
        nrm->set_parent (v.get_id());
        nrm->finalize();
        v.addVisualModel (nrm);
    }

    v.keepOpen();
}
