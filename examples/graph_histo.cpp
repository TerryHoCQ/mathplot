/*
 * Histogram example
 */

#include <iostream>
#include <sm/mathconst>
#include <sm/vec>
#include <sm/vvec>
#include <sm/histo>
#include <mplot/Visual.h>
#include <mplot/GraphVisual.h>

int main()
{
    // Find the distribution of the values of 1000 * sin(x) for 0 <= x <= 2pi (in 1000 steps)
    sm::vvec<float> numbers (1000);
    numbers.linspace (0.0f, sm::mathconst<float>::two_pi);
    for (auto& num : numbers) { num = 1000.0f * std::sin (num); }

    // Convert our numbers into a vvec of ints, to show that histogram can count up
    // ints, floats, doubles and so on.
    sm::vvec<int> inumbers = numbers.as<int>();

    // Make a histogram of the values of 1000 sin(x) with 30 bins. The first template
    // argument is for the type of the elements that will be counted up. The second
    // template arg is the floating point type to use to compute proportions (bin
    // positions, width, etc). This is float by default (which should be fine in most
    // cases), but is left explicit in this example.
    sm::histo<int, float> h(inumbers, 30);
#if 0
    // Setting a manual datarange can be useful if you are comparing histograms with different data:
    sm::histo<int, float> h(inumbers, 30, sm::range<int>{-2000, 2000});
#endif

    // Set up a mplot::Visual for a graph
    mplot::Visual v(1024, 768, "Histogram");
    v.setSceneTrans (sm::vec<float,3>({-0.539211f, -0.401911f, -2.8f}));

    // Create a new GraphVisual with offset within the scene of 0,0,0. Note the type for
    // the GraphVisual has to match the *second* template type for the histo.
    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>({0,0,0}));
    v.bindmodel (gv);
    gv->setdata (h); // to become gv->add_bargraph (h [,mplot::colour::darkorchid1] [,mplot::colour::orchid2])
    gv->xlabel = "1000 sin(x)";
    gv->ylabel = "Proportion";
    gv->finalize();
    v.addVisualModel (gv);

    // Render the graph until user exits
    v.keepOpen();

    return 0;
}
