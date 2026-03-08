/*
 * Visualize a Rod
 */
#include <iostream>
#include <memory>

import sm.vec;

import mplot.visual;
import mplot.gl.version;
import mplot.colourmap;
import mplot.rodvisual;

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

    constexpr sm::vec<float, 3> colour1 = { 1.0, 0.0, 0.0 };
    constexpr sm::vec<float, 3> colour2 = { 0.0, 0.9, 0.4 };

    std::cout << "creating first...\n" << std::flush;

    sm::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
    sm::vec<float, 3> start = { 0, 0, 0 };
    sm::vec<float, 3> end = { 0.25, 0, 0 };
    std::unique_ptr<mplot::VisualModel<>> rvm = std::make_unique<mplot::RodVisual<>> (offset, start, end, 0.1f, colour1, colour1);
    rvm->set_parent (v.get_id()); // A bind model call. howeer, we could pass v.visual_id to the VisualModel constructor
    rvm->finalize();
    std::cout << "addVisualmodel...\n" << std::flush;
    v.addVisualModel (rvm);

    sm::vec<float, 3> start2 = { -0.1, 0.2, 0.6 };
    sm::vec<float, 3> end2 = { 0.2, 0.4, 0.6 };
    // You can reuse the unique_ptr rvm, once you've transferred ownership with v.addVisualModel (rvm)
    rvm = std::make_unique<mplot::RodVisual<>>(offset, start2, end2, 0.05f, colour2);
    rvm->set_parent (v.get_id());
    rvm->finalize();
    std::cout << "addVisualmodel...\n" << std::flush;
    v.addVisualModel (rvm);

    v.keepOpen();

    return rtn;
}
