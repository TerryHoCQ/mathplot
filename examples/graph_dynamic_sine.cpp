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

    gv->setdata (x, (x+dx).sin());
    gv->finalize();

    auto gvp = v.addVisualModel (gv);

    while (v.readyToFinish() == false) {
        dx += 0.01;
        v.waitevents (0.01667); // 16.67 ms ~ 60 Hz
        gvp->update (x, (x+dx).sin(), 0);
        v.render();
    }

    return 0;
}
