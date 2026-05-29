/*
 * Visualize an Icosahedron
 */
#include <memory>

import mplot.visual;
import mplot.icosavisual;

int main()
{
    mplot::Visual v(1024, 768, "Icosahedron");
    v.showCoordArrows (true);
    // Switch on a mix of diffuse/ambient lighting
    v.lightingEffects (true);

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };
    sm::vec<float, 3> colour1 = { 1.0f, 0.0f, 0.0f };

    auto iv = std::make_unique<mplot::IcosaVisual<>> (offset, 0.9f, colour1);
    iv->set_parent (v.get_id());
    iv->finalize();
    v.addVisualModel (iv);

    v.keepOpen();
}
