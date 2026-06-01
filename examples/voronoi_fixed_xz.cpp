/*
 * Make a very small Voronoi surface where it makes sense to plot with surface heights
 * in the y direction. Demonstrates setting of VoronoiVisual::data_z_direction
 */
#include <memory>
#include <string>
#include <limits>
#include <vector>

import mplot.visual;
import mplot.voronoivisual;
import mplot.vectorvisual;

int main()
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "VoronoiVisual");

    std::vector<sm::vec<float>> points = {
        {0,1,0},
        {1,1,0},
        {0,1,1},
        {1,1,1},
        {.5,.8,0.5},
    };
    std::vector<float> data = {1,2,3,4,5};

    sm::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<mplot::VoronoiVisual<float>> (offset);
    vorv->set_parent (v.get_id());
    vorv->show_voronoi2d = true;
    vorv->debug_edges = true;
    vorv->debug_dataCoords = true;
    vorv->data_z_direction = sm::vec<float>{0,1,0};
    vorv->setDataCoords (&points);
    vorv->setScalarData (&data);
    vorv->finalize();
    auto p_vorv = v.addVisualModel (vorv);

    offset[0] -= 0.5f;
    auto vvm = std::make_unique<mplot::VectorVisual<float, 3>>(offset);
    vvm->set_parent (v.get_id());
    vvm->thevec = p_vorv->data_z_direction;
    vvm->fixed_colour = true;
    vvm->thickness = 0.03f;
    vvm->single_colour = mplot::colour::dodgerblue2;
    vvm->addLabel ("Arrow gives data direction", {-0.8, -0.3, 0.2}, mplot::TextFeatures(0.1f));
    vvm->finalize();
    v.addVisualModel (vvm);

    v.keepOpen();

    return rtn;
}
