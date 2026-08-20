/*
 * Visualize a plane
 */
#include <memory>
#include <iostream>

#include <array>
#include <stdexcept>
#include <string>

import mplot.visual;
import mplot.colour;
import mplot.planevisual;

int main()
{
    mplot::Visual v(1024, 768, "mplot::PlaneVisual");

    sm::vec<float> offset = {1,0,0};

    auto pvm = std::make_unique<mplot::PlaneVisual<>>(offset);
    pvm->set_parent (v.get_id());
    pvm->normal = {0,0,1};
    pvm->colour = mplot::colour::crimson;
    pvm->finalize();
    v.addVisualModel (pvm);

    v.keepOpen();
}
