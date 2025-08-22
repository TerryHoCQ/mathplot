#include <iostream>
#include <format>
#include <sm/random>
#include <sm/histo>
#include <mplot/Visual.h>
#include <mplot/GraphVisual.h>

int main (int argc, char** argv)
{
    using T = float;

    T mu = 0.0;
    T kappa = 3.0;
    if (argc > 1) { kappa = std::atof (argv[1]); }

    sm::rand_vonmises<T> rvm (mu, kappa);

    constexpr unsigned int nsamp = 100000;
    sm::vvec<T> samples (nsamp);
    for (unsigned int i = 0; i < nsamp; ++i) { samples[i] = rvm.get(); }

    sm::histo<T, float> h(samples, 100, sm::range<T>{-sm::mathconst<T>::pi, sm::mathconst<T>::pi});

#ifndef __APPLE__ // rand_vonmises::prob_density is not available with Apple, as it doesn't have std::cyl_bessel_i
    // Compute PD function
    sm::vvec<float> pd (h.bins.size());
    for (size_t i = 0; i < h.bins.size(); ++i) {
        pd[i] = static_cast<float>(rvm.prob_density (static_cast<T>(h.bins[i])));
    }
#endif

    // Set up a mplot::Visual for a graph
    mplot::Visual v(1024, 768, "Von Mises Distribution on the circle");
    v.setSceneTrans (sm::vec<float,3>{ float{-0.439335}, float{-0.472138}, float{-2.9} });

    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>{0,0,0});
    v.bindmodel (gv);

    gv->setdata (h, "", mplot::histo_view::densities);

#ifndef __APPLE__ // rand_vonmises::prob_density is not available with Apple, as it doesn't have std::cyl_bessel_i
    mplot::DatasetStyle ds (mplot::stylepolicy::lines);
    ds.linecolour = mplot::colour::crimson;
    gv->setdata (h.bins, pd, ds);
#else
    std::cout << "Info: you don't get to see the red probability density line on a Mac\n";
#endif
    gv->xlabel = "Angle";
    gv->ylabel = "Prob. density";
    gv->addLabel (std::format ("{}={}, {}={}",
                               mplot::unicode::toUtf8 (mplot::unicode::mu), mu,
                               mplot::unicode::toUtf8 (mplot::unicode::kappa), kappa),
                  sm::vec<float>{0, 1.1, 0}, mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();
}
