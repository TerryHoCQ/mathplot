/*
 * Linear regression, with visualisation
 */
#include <memory>
#include <iostream>

import sm.algo;

import mplot.visual;
import mplot.graphvisual;

int main()
{
    // Data
    sm::vvec<float> absc = { 1, 2, 3, 4, 5 }; // x
    sm::vvec<float> ord = { 1, 3, 2, 3, 5 }; // y

    // Fit y = mx + c
    sm::vec<float, 2> mc = sm::algo::linregr (absc, ord);
    std::cout << "Linear regression coefficients: gradient=" << mc[0] << ", offset=" << mc[1] << std::endl;
    // Create fit data points for visualisation:
    sm::vvec<float> fit = (absc * mc[0]) + mc[1];

    // Visualise data and linear fit
    mplot::Visual v(1024, 768, "Linear regression");
    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>{0,0,0});
    gv->set_parent (v.get_id());

    // The first dataset shows the data points
    mplot::DatasetStyle ds(mplot::stylepolicy::markers);
    ds.markercolour = mplot::colour::blue3;
    ds.markersize = 0.05;
    ds.datalabel = "data";
    gv->setdata (absc, ord, ds);

    // The second dataset is for the fit
    mplot::DatasetStyle ds2(mplot::stylepolicy::lines);
    ds2.linecolour = mplot::colour::lightsteelblue2;
    ds2.datalabel = "fit";
    gv->setdata (absc, fit, ds2);

    // Add axis labels, finalize and add to our mplot::Visual:
    gv->xlabel = "x";
    gv->ylabel = "y";
    gv->finalize();
    v.addVisualModel (gv);

    // Render the graph until user exits
    v.keepOpen();

    return 0;
}
