/*
 * Trace around a boundary of points in 2D
 */
#include <iostream>

#include <sm/mathconst>
#include <sm/random>
#include <sm/vvec>
#include <sm/vec>
#include <sm/algo>

#include <mplot/Visual.h>
#include <mplot/GraphVisual.h>


int main()
{
    // Get ready to graph the data
    mplot::Visual v(1024, 768, "Boundary tracing");
    mplot::DatasetStyle ds (mplot::stylepolicy::markers);
    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>{-0.5f,-0.5f,0.0f});
    v.bindmodel (gv);

    constexpr unsigned int n_points = 200;

    // Create data
    sm::rand_uniform<float> rngxy(-0.8f, 0.8f, n_points);
    sm::vvec<sm::vec<float, 2>> points(n_points);
    for (unsigned int i = 0; i < n_points; ++i) { points[i] = { rngxy.get(), rngxy.get() }; }

    // Trace algorithm
    sm::vvec<sm::vec<float, 2>> boundary = sm::geometry::graham_scan (points);

    gv->setlimits (-1.5, 1.5, -1.5, 1.5);

    gv->setdata (points, ds);

    ds.showlines = true;
    ds.linecolour = mplot::colour::crimson;
    ds.markercolour = mplot::colour::black;
    ds.markerstyle = mplot::markerstyle::circle;
    ds.markersize *= 1.2f;
    gv->setdata (boundary, ds);

    gv->finalize();
    v.addVisualModel (gv);
    v.keepOpen();
}
