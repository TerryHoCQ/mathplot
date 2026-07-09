#include <memory>

import mplot.colourmap;
import mplot.visual;
import mplot.rhombovisual;

int main()
{
    // Create a scene
    mplot::Visual v(1024, 768, "Rhombohedrons");
    //v.coordArrowsInScene (true);
    v.fov = 40;
    v.lightingEffects(false);

    // Parameters of the model
    sm::vec<float, 3> offset = { -1,  0,  0 };   // a within-scene offset
    sm::vec<float, 3> e1 = { 0.25,  0,  0 };
    sm::vec<float, 3> e2 = { 0.1,  0.25,  0 };
    sm::vec<float, 3> e3 = { 0,  0.0,  0.25 };
    mplot::ColourMap<float> cmap(mplot::ColourMapType::Rainbow);

    offset = { -2, 0, 0.05 };
    auto rv = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(1.0f));
    rv->set_parent (v.get_id());
    rv->finalize();
    v.addVisualModel (rv);

    offset = { 2, 0, -1.7 };
    auto rv2 = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.5f));
    rv2->set_parent (v.get_id());
    rv2->finalize();
    v.addVisualModel (rv2);

    offset = { 0, 2, 0.15 };
    auto rv3 = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.3333f));
    rv3->set_parent (v.get_id());
    rv3->finalize();
    v.addVisualModel (rv3);

    offset = { 2, 2, 0.5 };
    auto rv4 = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.25f));
    rv4->set_parent (v.get_id());
    rv4->finalize();
    v.addVisualModel (rv4);

    offset = { 0, -2.2, 0.9 };
    auto rv5 = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.2f));
    rv5->set_parent (v.get_id());
    rv5->finalize();
    v.addVisualModel (rv5);

    offset = { 0, -1.8, 1.7 };
    auto rv6 = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.1f));
    rv6->set_parent (v.get_id());
    rv6->finalize();
    v.addVisualModel (rv6);
    v.render();

    v.keepOpen();

    return 0;
}
