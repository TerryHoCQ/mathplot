// A scene of rhombohedrons useful for developing scene rotation by user control

#include <sm/vec>
#include <mplot/compoundray/Visual.h>
#include <mplot/RhomboVisual.h>
#include <mplot/ColourMap.h>

int main()
{
    // Create a scene
    mplot::compoundray::Visual v(1024, 768, "Rhombohedrons");
    v.lightingEffects (true);

    // Parameters of the model
    sm::vec<float, 3> offset = { -1,  0,  0 };   // a within-scene offset
    sm::vec<float, 3> e1 = { 0.25,  0,  0 };
    sm::vec<float, 3> e2 = { 0.1,  0.25,  0 };
    sm::vec<float, 3> e3 = { 0,  0.0,  0.25 };
    mplot::ColourMap<float> cmap(mplot::ColourMapType::Rainbow);

    // 0
    offset = { 0, 0, 0 };
    auto rv = std::make_unique<mplot::RhomboVisual<>> (offset, e1/3, e2/3, e3/3, cmap.convert(0.1f));
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);

    offset = { -2, 0, 0 };
    rv = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(1.0f));
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);

    offset = { 2, 0, 0 };
    rv = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.5f));
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);

    offset = { 0, 2, 0 };
    rv = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.3333f));
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);

    offset = { 0, -2, 0 };
    rv = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.25f));
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);

    offset = { 0, 0, 2 };
    rv = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.2f));
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);

    offset = { 0, 0, -2 };
    rv = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, cmap.convert(0.1f));
    v.bindmodel (rv);
    rv->finalize();
    v.addVisualModel (rv);
    v.render();

    v.keepOpen();

    return 0;
}
