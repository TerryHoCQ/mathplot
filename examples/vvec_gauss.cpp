// Compute a Gaussian with a vvec
#include <memory>
import mplot.visual;
import mplot.graphvisual;

int main()
{
    sm::vvec<double> x;
    double sigma = 1.5;
    unsigned int nsigma = 3;
    double hw = sigma * nsigma;
    x.linspace (-hw, hw, 60);
    sm::vvec<double> y = x.gauss (sigma);

    // Graph x and y
    mplot::Visual v(1024, 768, "1D convolutions with sm::vvec");
    auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{0,0,0});
    gv->set_parent (v.get_id());
    gv->setdata (x, y, "gauss");
    gv->finalize();
    v.addVisualModel (gv);
    v.keepOpen();

    return 0;
}
