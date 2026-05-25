/*
 * Visualize a Cone
 */
#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <memory>

import sm.vec;
import mplot.visual;
import mplot.conevisual;

int main()
{
    int rtn = 0;

    mplot::Visual v(1024, 768, "A simple cone");
    v.lightingEffects(true);

    try {
        sm::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        // Draw several cones, demonstrating what 'ringoffset' does
        for (int i = 0; i < 6; ++i) {
            auto cvm = std::make_unique<mplot::ConeVisual<>> (offset);
            cvm->set_parent (v.get_id());
            cvm->ringoffset = 0.2f * i;
            cvm->clr = { (5-i) * 0.2f, 0.0f, i * 0.2f };
            std::stringstream ss;
            ss << "ringoffset = " << cvm->ringoffset;
            cvm->addLabel (ss.str(), { 0.0f, 0.3f, 0.0f }, mplot::TextFeatures(0.05f));
            cvm->finalize();
            v.addVisualModel (cvm);

            offset[1] += 0.75f;
        }
        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
