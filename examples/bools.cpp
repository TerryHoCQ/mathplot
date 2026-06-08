#include <memory>

import mplot.visual;
import mplot.boolvisual;

int main()
{
    mplot::Visual v(1600, 1000, "BoolVisuals to show boolean state");
    v.lightingEffects();

    // A BoolVisual is a little indicator model to show a value.
    auto bv = std::make_unique<mplot::BoolVisual<>> (sm::vec<float>{-1, 0, 0});
    bv->set_parent (v.get_id());
    bv->width = 0.3;
    bv->tcol = mplot::colour::limegreen;
    bv->fcol = mplot::colour::grey20;
    bv->lbl = "20 > 10?";
    bv->value = (20 > 10 ? true : false);
    bv->finalize();
    v.addVisualModel (bv);

    bv = std::make_unique<mplot::BoolVisual<>> (sm::vec<float>{0, 0, 0});
    bv->set_parent (v.get_id());
    bv->width = 0.3;
    bv->tcol = mplot::colour::limegreen;
    bv->fcol = mplot::colour::grey20;
    bv->lbl = "20 < 10?";
    bv->value = (20 < 10 ? true : false);
    bv->finalize();
    v.addVisualModel (bv);

    // You can change the value and re-display with:
    //
    // auto bvp = v.addVisualModel (bv);
    // bvp->value = false;
    // bvp->reinitColours();
    // ...then later:
    // bvp->value = true;
    // bvp->reinitColours();

    v.keepOpen();
}
