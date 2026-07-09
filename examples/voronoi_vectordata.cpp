/*
 * Voronoi example using vector data and ColourMapType::RGB
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

    mplot::Visual v(1024, 768, "VoronoiVisual vectorData");

    sm::rand_uniform<float> rngxy(-2.0f, 2.0f, 1000);
    sm::rand_uniform<float> rngz(0.8f, 1.0f, 1000);

    // make n_points random coordinates
    std::vector<sm::vec<float>> points(n_points);
    std::vector<sm::vec<float>> data(n_points);

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), rngz.get() };
        // Set data RGB values from xyx position of points[i]
        data[i][0] = 0.5f + points[i][0] / 4.0f;
        data[i][1] = 0.5f + points[i][1] / 4.0f;
        data[i][2] = (-0.8f + points[i][2]) * 5.0f;
    }

    sm::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<mplot::VoronoiVisual<float>> (offset);
    vorv->set_parent (v.get_id());
    vorv->show_voronoi2d = true; // true to show the 2D voronoi edges
    vorv->debug_dataCoords = false; // true to show coordinate spheres
    float length_scale = 4.0f / std::sqrt (n_points);
    vorv->border_width  = length_scale;
#if 1
    // With RGB, the input is passed in as RGB channels with each channel being in range [0, 1]
    vorv->cm.setType (mplot::ColourMapType::RGB);
#else
# if 0
    // You can alternatively specify a 2D map like DiscFourBlack...
    vorv->cm.setType (mplot::ColourMapType::DiscFourBlack);
# else
    // ...or a 1D map with 'act as if 2d' set true (which will desaturate the map with one dimension of the data):
    vorv->cm.setType (mplot::ColourMapType::Plasma);
    vorv->cm.set_act_2d (true);
# endif
#endif

    vorv->setDataCoords (&points);
    vorv->setVectorData (&data);
    vorv->finalize();
    v.addVisualModel (vorv);

    v.keepOpen();

    return rtn;
}
