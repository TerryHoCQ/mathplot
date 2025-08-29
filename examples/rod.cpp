/*
 * Visualize a Rod
 */
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <stdexcept>
#include <string>

#include <sm/vec>

#include <mplot/Visual.h>
#include <mplot/ColourMap.h>
#include <mplot/RodVisual.h>

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
        constexpr sm::vec<float, 3> colour2 = { 0.0, 0.9, 0.4 };

        sm::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        sm::vec<float, 3> start = { 0, 0, 0 };
        sm::vec<float, 3> end = { 0.25, 0, 0 };
        std::unique_ptr<mplot::VisualModel<>> rvm = std::make_unique<mplot::RodVisual<>> (offset, start, end, 0.1f, colour1, colour1);
        v.bindmodel (rvm);
        rvm->finalize();
        v.addVisualModel (rvm);

        sm::vec<float, 3> start2 = { -0.1, 0.2, 0.6 };
        sm::vec<float, 3> end2 = { 0.2, 0.4, 0.6 };
        // You can reuse the unique_ptr rvm, once you've transferred ownership with v.addVisualModel (rvm)
        rvm = std::make_unique<mplot::RodVisual<>>(offset, start2, end2, 0.05f, colour2);
        v.bindmodel (rvm);
        rvm->finalize();
        v.addVisualModel (rvm);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
