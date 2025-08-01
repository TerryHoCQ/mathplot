#include <iostream>
#include <format>
#include <sm/random>
#include <sm/histo>
#include <mplot/Visual.h>
#include <mplot/GraphVisual.h>

int main (int argc, char** argv)
{
    double mu = 0.0;
    double kappa = 3.0;
    if (argc > 1) { kappa = std::atof (argv[1]); }

    sm::rand_vonmises<double> rvm (mu, kappa);

    constexpr unsigned int nsamp = 100000;
    sm::vvec<double> samples (nsamp);
    for (unsigned int i = 0; i < nsamp; ++i) { samples[i] = rvm.get(); }

    sm::histo<double, float> h(samples, 100, sm::range<double>{-sm::mathconst<double>::pi, sm::mathconst<double>::pi});

    // Set up a mplot::Visual for a graph
    mplot::Visual v(1024, 768, "Von Mises Distribution on the circle");
    v.setSceneTrans (sm::vec<float,3>{ float{-0.439335}, float{-0.472138}, float{-2.9} });

    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>{0,0,0});
    v.bindmodel (gv);
    gv->setdata (h);
    gv->xlabel = "Angle";
    gv->ylabel = "Proportion";
    gv->addLabel (std::format ("{}={}, {}={}",
                               mplot::unicode::toUtf8 (mplot::unicode::mu), mu,
                               mplot::unicode::toUtf8 (mplot::unicode::kappa), kappa),
                  sm::vec<float>{0, 1.1, 0}, mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();
}
