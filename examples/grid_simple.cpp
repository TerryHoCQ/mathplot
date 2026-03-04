/*
 * An example mplot::Visual scene, containing a Gridv, and using GridvVisual
 */

#include <iostream>
#include <vector>
#include <cmath>

import sm.vec;
import sm.grid;

#include <mplot/Visual.h>
#include <mplot/VisualDataModel.h>
#include <mplot/GridVisual.h>

int main()
{
    mplot::Visual v(1600, 1000, "mplot::GridVisual");

#ifdef ORTHOGRAPHIC
    // Here's how to set your Visual to do an orthographic projection rather than perspective
    v.ptype = mplot::perspective_type::orthographic;
#endif

    // Create a grid to show in the scene
    constexpr unsigned int Nside = 100;
    constexpr sm::vec<float, 2> grid_spacing = {0.01f, 0.01f};

    // The simplest declaration of Grid is:
    //   sm::Grid g(size_t n_x, size_t n_y);
    // grid_spacing, grid_zero, use of memory, wrapping and ordering are all possible arguments to
    // the constructor.
    sm::grid grid(Nside, Nside, grid_spacing);

    std::cout << "Number of pixels in grid:" << grid.n() << std::endl;

    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(grid.n(), 0.0);
    for (unsigned int ri=0; ri<grid.n(); ++ri) {
        auto coord = grid[ri];
        data[ri] = 0.05f + 0.05f * std::sin(20.0f * coord[0]) * std::sin(10.0f*coord[1]) ; // Range 0->1
    }

    float step = 0.6f;
    // Add a GridVisual to display the Grid within the mplot::Visual scene
    sm::vec<float, 3> offset = { -step * grid.width(), -step * grid.width(), 0.0f };

    auto gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = mplot::GridVisMode::Triangles;
    gv->setScalarData (&data);
    gv->cm.setType (mplot::ColourMapType::Cork);
    gv->addLabel (std::string("GridVisMode::Triangles, cm: ") + gv->cm.getTypeStr(), sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.03f));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { step * grid.width(), -step * grid.width(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&data);

    // To show a grid first set the scaling so that the whole image is flat
    //gv->zScale.null_scaling();
    // Say you want a grid
    //gv->showgrid (true);
    // Choose the thickness. This is a proportion of the pixel dimensions
    //gv->grid_thickness = 0.1f;

    gv->cm.setType (mplot::ColourMapType::Buda);
    gv->addLabel (std::string("GridVisMode::RectInterp, cm: ") + gv->cm.getTypeStr(), sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.03f));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { -step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = mplot::GridVisMode::Columns;
    gv->interpolate_colour_sides (true);
    gv->setScalarData (&data);
    gv->cm.setType (mplot::ColourMapType::Imola);
    gv->addLabel (std::string("GridVisMode::Columns, interpolated sides, cm: ") + gv->cm.getTypeStr(), sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.03f));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = mplot::GridVisMode::Columns;
    //gv->interpolate_colour_sides = false; // default
    //gv->clr_east_column = mplot::colour::black; // These are defaults but you can change them
    //gv->clr_north_column = mplot::colour::black;
    gv->setScalarData (&data);
    gv->cm.setType (mplot::ColourMapType::Managua);
    gv->addLabel (std::string("GridVisMode::Columns, black sides, cm: ") + gv->cm.getTypeStr(), sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.03));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { 3 * step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = mplot::GridVisMode::Pixels;
    gv->setScalarData (&data);
    gv->cm.setType (mplot::ColourMapType::Navia);
    gv->addLabel (std::string("GridVisMode::Pixels, cm: ") + gv->cm.getTypeStr(), sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.03));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { 3 * step * grid.width(), -step * grid.width(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&data);
    gv->cm.setType (mplot::ColourMapType::Navia);
    gv->addLabel (std::string("GridVisMode::RectInterp, implygrid=true, cm: ") + gv->cm.getTypeStr(), sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.03));
    gv->implygrid (true);
    gv->grid_thickness = 0.18f; // of a pixel
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();

    return 0;
}
