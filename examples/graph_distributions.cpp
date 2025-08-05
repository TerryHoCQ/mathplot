/*
 * Showing all the random distributions in <sm/random>
 */

#include <iostream>
#include <format>
#include <string>
#include <sm/mathconst>
#include <sm/vec>
#include <sm/vvec>
#include <sm/histo>
#include <mplot/Visual.h>
#include <mplot/GraphVisual.h>


template<typename F>
void draw_graph (mplot::Visual<>& v, sm::histo<F, float>& h, const sm::vvec<float>& pd,
                 const sm::vec<float>& locn, const std::string label, const bool proportions = false)
{
    mplot::DatasetStyle ds (mplot::stylepolicy::lines);
    ds.linecolour = mplot::colour::crimson;
    // Create a new GraphVisual. Note the type for the GraphVisual has to match the *second*
    // template type for the histo.
    auto gv = std::make_unique<mplot::GraphVisual<float>> (locn);
    v.bindmodel (gv);
    if (!proportions) {
        gv->setdata (h, "samples", mplot::histo_view::densities);
        ds.datalabel = "PDF";
        gv->setdata (h.bins, pd, ds);
        gv->ylabel = std::format("Prob. density (sum {})", h.densities.sum());
    } else {
        gv->setdata (h, "samples");
        ds.datalabel = "Expected";
        gv->setdata (h.bins, pd, ds);
        gv->ylabel = std::format("Count proportions (sum {})", h.proportions.sum()) ;
    }
    gv->addLabel (label, sm::vec<>{0.5, 1.17, 0}, mplot::TextFeatures(0.055f, mplot::colour::black, true));
    gv->finalize();
    v.addVisualModel (gv);
}

int main()
{
    using F = float;
    namespace uc = mplot::unicode;
    constexpr float gw = 1.4f;
    constexpr float gh = 1.55f;
    constexpr std::size_t nbins = 100;
    constexpr unsigned int nsamp = 10000;
    sm::vvec<F> samples (nsamp);
    sm::vvec<float> pd (nbins);
    // Named parameters for distributions
    F a, b, alpha, mu, sigma, lambda, kappa;

    // Set up a mplot::Visual for a graph
    mplot::Visual v(2000, 1200, "RNG distributions available in <sm/random>");
    v.setSceneTrans (sm::vec<float,3>{ float{-2.49956}, float{0.362711}, float{-6.9} });

    // Uniform distribution
    a = F{0};
    b = F{10};
    sm::rand_uniform<F> uniform (a, b);
    for (unsigned int i = 0; i < nsamp; ++i) { samples[i] = uniform.get(); }
    sm::histo<F, float> h(samples, nbins);
    for (size_t i = 0; i < h.bins.size(); ++i) {
        pd[i] = static_cast<float>(uniform.prob_density (static_cast<F>(h.bins[i])));
    }
    draw_graph (v, h, pd, sm::vec<>{}, std::format("Uniform [{}, {}]", a, b));

    // Normal distribution
    mu = F{4};
    sigma = F{1};
    sm::rand_normal<F> normal (mu, sigma);
    for (unsigned int i = 0; i < nsamp; ++i) { samples[i] = normal.get(); }
    h.init (samples, nbins);
    for (size_t i = 0; i < h.bins.size(); ++i) {
        pd[i] = static_cast<float>(normal.prob_density (static_cast<F>(h.bins[i])));
    }
    draw_graph (v, h, pd, sm::vec<>{gw},
                std::format("Normal {}={}, {}={}", uc::toUtf8(uc::mu), mu, uc::toUtf8(uc::sigma), sigma));

    // Log-normal distribution
    mu = F{0.4};
    sigma = F{0.6};
    sm::rand_lognormal<F> lognormal (mu, sigma);
    for (unsigned int i = 0; i < nsamp; ++i) { samples[i] = lognormal.get(); }
    h.init (samples, nbins);
    for (size_t i = 0; i < h.bins.size(); ++i) {
        pd[i] = static_cast<float>(lognormal.prob_density (static_cast<F>(h.bins[i])));
    }
    draw_graph (v, h, pd, sm::vec<>{2 * gw},
                std::format("Log-normal {}={}, {}={}", uc::toUtf8(uc::mu), mu, uc::toUtf8(uc::sigma), sigma));

    // Poisson distribution
    mu = F{2};
    sm::rand_poisson<int> poisson (mu);
    for (unsigned int i = 0; i < nsamp; ++i) { samples[i] = poisson.get(); }
    h.init (samples, nbins);
    for (size_t i = 0; i < h.bins.size(); ++i) {
        pd[i] = static_cast<float>(poisson.prob_density (static_cast<F>(h.bins[i])));
    }
    draw_graph (v, h, pd, sm::vec<>{3 * gw},
                std::format("Poisson {}={}", uc::toUtf8(uc::mu), mu), true);

    // Exponential distribution
    lambda = F{2};
    sm::rand_exponential<F> exponential (lambda);
    for (unsigned int i = 0; i < nsamp; ++i) { samples[i] = exponential.get(); }
    h.init (samples, nbins);
    for (size_t i = 0; i < h.bins.size(); ++i) {
        pd[i] = static_cast<float>(exponential.prob_density (static_cast<F>(h.bins[i])));
    }
    draw_graph (v, h, pd, sm::vec<>{0, -gh},
                std::format("Exponential {}={}", uc::toUtf8(uc::lambda), lambda));

    // Pareto distribution
    alpha = F{2};
    sigma = F{1};
    sm::rand_pareto<F> pareto (alpha, sigma);
    for (unsigned int i = 0; i < nsamp; ++i) { samples[i] = pareto.get(); }
    h.init (samples, nbins);
    for (size_t i = 0; i < h.bins.size(); ++i) {
        pd[i] = static_cast<float>(pareto.prob_density (static_cast<F>(h.bins[i])));
    }
    draw_graph (v, h, pd, sm::vec<>{gw, -gh},
                std::format("Pareto {}={} {}={}", uc::toUtf8(uc::alpha), alpha, uc::toUtf8(uc::sigma), sigma));

#ifndef __APPLE__
    // Von Mises distribution
    mu = F{0};
    kappa = F{0.7};
    sm::rand_vonmises<F> vonmises (mu, kappa);
    for (unsigned int i = 0; i < nsamp; ++i) { samples[i] = vonmises.get(); }
    h.init (samples, nbins);
    for (size_t i = 0; i < h.bins.size(); ++i) {
        pd[i] = static_cast<float>(vonmises.prob_density (static_cast<F>(h.bins[i])));
    }
    draw_graph (v, h, pd, sm::vec<>{2 * gw, -gh},
                std::format("Von Mises {}={} {}={}", uc::toUtf8(uc::mu), mu, uc::toUtf8(uc::kappa), kappa));
#endif

    v.keepOpen();

    return 0;
}
