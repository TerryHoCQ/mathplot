/*
 * Visualize a triangle
 */
#include <iostream>
#include <memory>

import sm.vec;
import mplot.visual;
import mplot.trianglevisual;

int main()
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "Visualization");
    v.zNear = 0.001f;
    v.coordArrowsInScene (true);
    // For a white background:
    v.backgroundWhite();
    // Switch on a mix of diffuse/ambient lighting
    v.lightingEffects(true);

    try {
        sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

        sm::vec<float, 3> c1 = { 0.0f, 0.0f, 0.0f };
        sm::vec<float, 3> c2 = { 0.25f, 0.0f, 0.0f };
        sm::vec<float, 3> c3 = { 0.0f, 0.3f, 0.0f };

        sm::vec<float, 3> colour1 = { 1.0f, 0.0f, 0.0f };

        auto tv = std::make_unique<mplot::TriangleVisual<>> (offset, c1, c2, c3, colour1);
        tv->set_parent (v.get_id());
        tv->finalize();
        v.addVisualModel (tv);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
