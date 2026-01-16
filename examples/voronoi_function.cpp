/*
 * This example generates a number (n_points) of random xy positions. zcoords are some
 * function. The value of z is used also as the scalarData input, so that the colourmap
 * represents the height of the surface.c
 *
 * Author Seb James
 * Date 2024
 */
#include <iostream>
#include <sm/vec>
#include <sm/random>
#include <mplot/Visual.h>
#include <mplot/VoronoiVisual.h>

static constexpr int n_points = 1000;

int main()
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "VoronoiVisual");

    sm::rand_uniform<float> rngxy(-2.0f, 2.0f, 1000);

    // make n_points random coordinates
    std::vector<sm::vec<float>> points(n_points);
    std::vector<float> data(n_points);
    std::vector<float> r(n_points);

    float k = 1.0f;

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), 0.0f };
        r[i] = points[i].length();
        data[i] = std::sin (k * r[i]) / k * r[i]; // colour function
        points[i][2] = data[i];
    }

    mplot::ColourMapType cmap_t = mplot::ColourMapType::Plasma;

    sm::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<mplot::VoronoiVisual<float>> (offset);
    v.bindmodel (vorv);
    vorv->show_voronoi2d = false; // true to show the 2D voronoi edges
    vorv->debug_dataCoords = false; // true to show coordinate spheres
    float length_scale = 4.0f / std::sqrt (n_points);
    vorv->border_width = length_scale;
    //vorv->dom_shape = mplot::VoronoiVisual<float>::domain_shape::traced;
    //vorv->dom_shape = mplot::VoronoiVisual<float>::domain_shape::circular;
    //vorv->dom_shape = mplot::VoronoiVisual<float>::domain_shape::rectangular; // default
    vorv->cm.setType (cmap_t);
    vorv->setDataCoords (&points);
    vorv->setScalarData (&data);
    vorv->finalize();
    auto vorvp = v.addVisualModel (vorv);

    int fcount = 0;
    while (!v.readyToFinish()) {

        if (k > 8.0f) { k = 1.0f; }
        for (unsigned int i = 0; i < n_points; ++i) {
            data[i] = std::sin (k * r[i]) / k * r[i];
            points[i][2] = data[i];
        }

        // Periodically change the colourmap
        if (fcount++% 1800 == 0) { vorvp->cm.setType (++cmap_t); }

        vorvp->reinit(); // slow, but map has to be rebuilt

        v.waitevents(0.001);
        v.render();
        k += 0.01f;
    }

    return rtn;
}
