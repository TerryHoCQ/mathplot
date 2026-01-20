/*
 * Trace around a boundary of points in 2D. This code uses Graham's scan to compute the convex hull
 * of a set of randomly generated points.
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
    mplot::Visual v(1024, 768, "Graham's scan computes the convex hull");
    mplot::DatasetStyle ds (mplot::stylepolicy::markers);
    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>{-0.5f,-0.5f,0.0f});
    v.bindmodel (gv);

    constexpr unsigned int n_points = 200;

    // Create data
    sm::rand_uniform<float> rngxy(-0.8f, 0.8f);
    sm::vvec<sm::vec<float, 2>> points(n_points);
    for (unsigned int i = 0; i < n_points; ++i) { points[i] = { rngxy.get(), rngxy.get() }; }

    // Trace algorithm
    sm::vvec<sm::vec<float, 2>> boundary = sm::geometry::graham_scan (points);
    // Close the boundary
    boundary.push_back (boundary.front());

    gv->setlimits (-1, 1, -1, 1);

    gv->setdata (points, ds);

    mplot::DatasetStyle ds2 (mplot::stylepolicy::lines);
    ds2.linecolour = mplot::colour::crimson;
    gv->setdata (boundary, ds2);

    gv->finalize();
    v.addVisualModel (gv);
    v.keepOpen();
}
