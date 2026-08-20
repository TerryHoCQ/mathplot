/*
 * This example generates a number (n_points) of random xy positions. The z coordinate
 * is always 0. A function gives scalarData input, so that the colourmap represents the
 * value of the function.
 *
 * Here, we also show the Delaunay triangulation.
 *
 * Author Seb James
 * Date 2026
 */
#include <memory>
#include <vector>
#include <cmath>

import sm.random;
import mplot.colourmap;
import mplot.visual;
import mplot.voronoivisual;

static constexpr int n_points = 100;

int main()
{
    mplot::Visual v(1024, 768, "Voronoi cells and Delaunay Triangulation");

    sm::rand_uniform<float> rngxy(-2.0f, 2.0f, 1000);

    // make n_points random coordinates
    std::vector<sm::vec<float>> points(n_points);
    std::vector<float> data(n_points);
    std::vector<float> r(n_points);

    float k = 1.0f;

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), -0.1f };
        r[i] = points[i].length();
        data[i] = std::sin (k * r[i]) / k * r[i]; // colour function
    }

    mplot::ColourMapType cmap_t = mplot::ColourMapType::Plasma;

    sm::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<mplot::VoronoiVisual<float>> (offset);
    vorv->set_parent (v.get_id());
    vorv->show_voronoi2d = false;   // true to show the 2D voronoi edges
    vorv->show_delaunay2d = true;   // true to show Delaunay triangulation (the dual of the voronoi grid).
    vorv->debug_dataCoords = false; // true to show coordinate spheres
    float length_scale = 4.0f / std::sqrt (n_points);
    vorv->border_width = length_scale;
    vorv->dom_shape = mplot::VoronoiVisual<float>::domain_shape::traced;
    //vorv->dom_shape = mplot::VoronoiVisual<float>::domain_shape::circular;
    //vorv->dom_shape = mplot::VoronoiVisual<float>::domain_shape::rectangular; // default
    vorv->cm.setType (cmap_t);
    vorv->setDataCoords (&points);
    vorv->setScalarData (&data);
    vorv->finalize();
    v.addVisualModel (vorv);

    v.keepOpen();
}
