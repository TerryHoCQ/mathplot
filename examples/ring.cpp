/*
 * Visualize a ring with mplot::RingVisual
 */
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <memory>

import sm.vec;

import mplot.visual;
import mplot.ringvisual;
import mplot.colourmap;

int main()
{
    mplot::Visual v(1024, 768, "A ring");
    v.lightingEffects(true);
    mplot::ColourMap<float> cmap;
    sm::vec<int, 6> segs = {3, 4, 6, 8, 12, 24};
    sm::vec<float, 3> offset = { -6.0f, 0.0f, 0.0f };
    for (int i = 0; i < 6; ++i) {
        auto rvm = std::make_unique<mplot::RingVisual<>> (offset);
        rvm->set_parent (v.get_id());
        rvm->clr = cmap.convert (i/6.0f);
        rvm->segments = segs[i];
        rvm->finalize();
        v.addVisualModel (rvm);
        offset[0] += 2.3f;
    }
    v.keepOpen();

    return 0;
}
