// Compute a Gaussian with a vvec
#include <memory>
import mplot.visual;
import mplot.graphvisual;

int main()
{
    sm::vvec<float> rgauss (360, 0.0f);
    float sigma = 45.0f;
    rgauss.linspace (-180.0f, 179.0f, 360);
    rgauss.gauss_inplace (sigma);
    rgauss.rotate (180);
    sm::vvec<float> ggauss = rgauss;
    ggauss.rotate (-120);
    sm::vvec<float> bgauss = ggauss;
    bgauss.rotate (-120);

    // For x axis
    sm::vvec<float> x;
    x.linspace (0.0f, 359.0f, 360);

    // Graph x and y
    mplot::Visual v(1024, 768, "RGB");
    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>({0,0,0}));
    gv->set_parent (v.get_id());
    mplot::DatasetStyle ds(mplot::stylepolicy::markers);
    ds.datalabel = "R";
    ds.markercolour = mplot::colour::crimson;
    gv->setdata (x, rgauss, ds);

    ds.datalabel = "G";
    ds.markercolour = mplot::colour::springgreen;
    gv->setdata (x, ggauss, ds);

    ds.datalabel = "B";
    ds.markercolour = mplot::colour::royalblue;
    gv->setdata (x, bgauss, ds);

    gv->finalize();
    v.addVisualModel (gv);
    v.keepOpen();

    return 0;
}
