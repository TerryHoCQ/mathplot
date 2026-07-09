// This is a graph which updates on each step to make sure the x axis tick labelling works
#include <memory>
#include <cstdint>

import mplot.visual;
import mplot.graphvisual;

int main()
{
    mplot::Visual v(1024, 768, "Continuous redrawing of GraphVisual");

    auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{0,0,0});
    gv->set_parent (v.get_id());

    sm::vvec<double> x;
    x.linspace (-sm::mathconst<double>::pi, sm::mathconst<double>::pi, 100);

    double dx = 0.0;

    mplot::DatasetStyle ds_left;
    ds_left.datalabel = "sine left";
    gv->setdata (x, (x+dx).sin(), ds_left);
    gv->fontsize *= 2.0f; // Bigger fonts to encourage more font size auto-adjustment
    // Enable auto-rescaling of the x axis
    gv->auto_rescale_x = true;
    gv->auto_rescale_y = true;
    gv->finalize();

    auto gvp = v.addVisualModel (gv);

    double dx_step = 0.01;
    std::int64_t count = 0;
    double f = 1.0;
    while (v.readyToFinish() == false) {
        v.waitevents (0.016); // 16.67 ms ~ 60 Hz
        if (count++ % 60 == 0) {
            x *= 2.0;
            f /= 2.0;
        }
        dx += dx_step;
        gvp->update (x+dx, (f*x+dx).sin(), 0);
        v.render();
    }

    return 0;
}
