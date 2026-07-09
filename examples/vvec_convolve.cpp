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

    // Manually create a filter:
    sm::vvec<double> filter = {.2, .4, .6, .8, 1, 0.8, 0.6, 0.4, 0.2};
    filter /= filter.sum();

    sm::vvec<double> y2 = y.convolve<wrapdata::wrap> (filter);

    // Graph x and y
    mplot::Visual v(1024, 768, "1D convolutions with sm::vvec");
    auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{0,0,0});
    gv->set_parent (v.get_id());
    gv->setdata (x, y, "raw");
    gv->setdata (x, y2, "smth");
    gv->finalize();
    v.addVisualModel (gv);
    v.keepOpen();

    return 0;
}
