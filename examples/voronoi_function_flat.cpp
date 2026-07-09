/*
 * This example generates a number (n_points) of random xy positions. The z coordinate
 * is always 0. A function gives scalarData input, so that the colourmap represents the
 * value of the function.
 *
 * Author Seb James
 * Date 2024
 */
#include <memory>
#include <vector>
#include <cmath>

import sm.random;
import mplot.colourmap;
import mplot.visual;
import mplot.voronoivisual;

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
    }

    mplot::ColourMapType cmap_t = mplot::ColourMapType::Plasma;

    sm::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<mplot::VoronoiVisual<float>> (offset);
    vorv->set_parent (v.get_id());
    vorv->show_voronoi2d = true; // true to show the 2D voronoi edges
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
        }

        if (fcount++% 600 == 0) {
            vorvp->cm.setType (++cmap_t);
        }
        vorvp->reinitColours();
        v.waitevents(0.018);
        v.render();
        k += 0.01f;
    }

    return rtn;
}
