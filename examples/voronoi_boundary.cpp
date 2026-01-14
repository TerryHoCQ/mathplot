/*
 * This example generates a number (n_points) of random (but bounded) coordinates and
 * uses the VoronoiVisual to display the coordinates as a map, with the order of
 * random-choice being used to colourize the Voronoi cells.
 *
 * Test harness for drawing an arbitrary boundary around the Voronoi region
 *
 * Author Seb James
 * Date 2026
 */
#include <iostream>
#include <sm/vec>
#include <sm/random>
#include <mplot/Visual.h>
#include <mplot/VoronoiVisual.h>

static constexpr int n_points = 80;

int main()
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "VoronoiVisual");

    sm::rand_uniform<float> rngxy(-0.8f, 0.8f, n_points);
    sm::rand_uniform<float> rngz(0.8f, 1.0f, n_points);

    // make n_points random coordinates
    std::vector<sm::vec<float>> points(n_points);
    std::vector<float> data(n_points);

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), rngz.get() };
        data[i] = static_cast<float>(i) / n_points;
    }

    mplot::ColourMapType cmap_t = mplot::ColourMapType::Plasma;

    sm::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<mplot::VoronoiVisual<float>> (offset);
    v.bindmodel (vorv);
    vorv->show_voronoi2d = true; // true to show the 2D voronoi edges
    vorv->debug_dataCoords = true; // true to show coordinate spheres
#if 0
    // Use a rectangular domain boundary
    // vorv->rectangular_domain = true; // this is default
    // vorv->border_width = 0.5f;
#else
    // polygonal
    //vorv->dom_shape = mplot::VoronoiVisual<float>::domain_shape::circular;
    vorv->dom_shape = mplot::VoronoiVisual<float>::domain_shape::ellipsoid;
    vorv->num_boundary_points = 30;   // default 30
    vorv->border_width = 0.01f;
#endif
    vorv->cm.setType (cmap_t);
    vorv->setDataCoords (&points);
    vorv->setScalarData (&data);
    vorv->finalize();
    v.addVisualModel (vorv);

    v.keepOpen();

    return rtn;
}
