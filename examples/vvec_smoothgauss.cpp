// Example of 1D convolutions with vvec
#include <memory>
import mplot.visual;
import mplot.graphvisual;

int main()
{
    using mc = sm::mathconst<double>;
    using wrapdata = sm::vvec<double>::wrapdata;

    sm::vvec<double> x;
    x.linspace (-mc::pi, mc::pi-(mc::pi/5.0), 60);
    sm::vvec<double> y = x.sin();
    sm::vvec<double> r (x.size(), 0.0);
    r.randomize();
    y += r;

    sm::vvec<double> y2 = y.smooth_gauss<wrapdata::wrap> (3.0, 3);

    wrapdata w = wrapdata::wrap;
    sm::vvec<double> yd = y2;
    yd.diff_inplace (w);
    yd*=1;

    sm::vvec<double> yd2 = y2.diff (w);
    yd2 += 0.5;


    // Graph x and y
    mplot::Visual v(1024, 768, "Gaussian smoothing with sm::vvec");
    auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>({0,0,0}));
    gv->set_parent (v.get_id());
    gv->setdata (x, y, "raw");
    gv->setdata (x, y2, "smth");
    gv->setdata (x, yd, "smthdiff inplace");
    gv->setdata (x, yd2, "smthdiff");
    gv->finalize();
    v.addVisualModel (gv);
    v.render();
    v.keepOpen();

    return 0;
}
