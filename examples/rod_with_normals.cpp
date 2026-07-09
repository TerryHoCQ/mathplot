/*
 * Visualize a Rod
 */
#include <memory>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <stdexcept>
#include <string>

import mplot.visual;
import mplot.colourmap;
import mplot.rodvisual;
import mplot.normalsvisual;

int main()
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "Visualization");
    v.zNear = 0.001;
    //v.showCoordArrows (true);
    //v.coordArrowsInScene (true);
    // For a white background:
    v.backgroundWhite();
    // Switch on a mix of diffuse/ambient lighting
    v.lightingEffects(true);

    try {
        constexpr sm::vec<float, 3> colour1 = { 1.0, 0.0, 0.0 };
        sm::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        sm::vec<float, 3> start = { 0, 0, 0 };
        sm::vec<float, 3> end = { 0.25, 0, 0 };
        auto rvm = std::make_unique<mplot::RodVisual<>> (offset, start, end, 0.1f, colour1, colour1);
        rvm->set_parent (v.get_id());
        rvm->use_oriented_tube = false;
        rvm->finalize();
        auto rvmp1 = v.addVisualModel (rvm);

        auto nrm = std::make_unique<mplot::NormalsVisual<>> (rvmp1);
        nrm->set_parent (v.get_id());
        nrm->finalize();
        v.addVisualModel (nrm);

        constexpr sm::vec<float, 3> colour2 = { 0.0, 0.9, 0.4 };
        sm::vec<float, 3> start2 = { -0.1, 0.2, 0.6 };
        sm::vec<float, 3> end2 = { 0.2, 0.4, 0.6 };
        // You can reuse the unique_ptr rvm, once you've transferred ownership with v.addVisualModel (rvm)
        rvm = std::make_unique<mplot::RodVisual<>>(offset, start2, end2, 0.05f, colour2);
        rvm->set_parent (v.get_id());
        rvm->finalize();
        auto rvmp2 = v.addVisualModel (rvm);

        // Create an associate normals model
        nrm = std::make_unique<mplot::NormalsVisual<>> (rvmp2);
        nrm->set_parent (v.get_id());
        nrm->finalize();
        v.addVisualModel (nrm);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
