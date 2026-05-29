// This is a graph which updates on each step. To test for a bug, but also to show how
// a graph can be completely redrawn each time, if required.
#include <memory>

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

    mplot::DatasetStyle ds_right;
    ds_right.axisside = mplot::axisside::right;
    ds_right.linecolour = mplot::colour::red2;
    ds_right.markercolour = mplot::colour::red2;
    ds_right.datalabel = "sine right";
    gv->setdata (x, (x+dx).sin() -0.5, ds_right);

    // set style of the axis
    gv->axisstyle = mplot::axisstyle::twinax;

    // Enable auto-rescaling of the x axis
    gv->auto_rescale_x = true;
    // Enable auto-rescaling of the y axis
    gv->auto_rescale_y = true;

    gv->finalize();

    auto gvp = v.addVisualModel (gv);

    while (v.readyToFinish() == false) {
        dx += 0.01;
        v.waitevents (0.01667); // 16.67 ms ~ 60 Hz
        gvp->update (x+dx, (x+dx).sin() + dx, 0);
        gvp->update (x-dx, (x+dx).sin() - dx - 0.5, 1);
        v.render();
    }

    return 0;
}
