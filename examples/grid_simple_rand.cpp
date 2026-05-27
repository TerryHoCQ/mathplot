/*
 * An example mplot::Visual scene, containing a Gridv, and using GridvVisual
 */

#include <memory>
#include <iostream>

import sm.vec;
import sm.vvec;
import sm.grid;

import mplot.visual;
import mplot.gridvisual;

int main()
{
    mplot::Visual v(1600, 1000, "mplot::GridVisual");

#ifdef ORTHOGRAPHIC
    // Here's how to set your Visual to do an orthographic projection rather than perspective
    v.ptype = mplot::perspective_type::orthographic;
#endif

    // Create a grid to show in the scene
    constexpr unsigned int Nside = 10;
    constexpr sm::vec<float, 2> grid_spacing = {0.1f, 0.1f};

    // The simplest declaration of Grid is:
    //   sm::Grid g(size_t n_x, size_t n_y);
    // grid_spacing, grid_zero, use of memory, wrapping and ordering are all possible arguments to
    // the constructor.
    sm::grid grid(Nside, Nside, grid_spacing);

    std::cout << "Number of pixels in grid:" << grid.n() << std::endl;

    // Make some dummy data (random values) to make an interesting surface
    sm::vvec<float> data(grid.n());
    data.randomize();

    float step = 0.6f;
    // Add a GridVisual to display the Grid within the mplot::Visual scene
    sm::vec<float, 3> offset = { -step * grid.width(), -step * grid.width(), 0.0f };

    auto gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::Triangles;
    gv->setScalarData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->addLabel ("GridVisMode::Triangles", sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { step * grid.width(), -step * grid.width(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->addLabel ("GridVisMode::RectInterp", sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { -step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::Columns;
    gv->interpolate_colour_sides (true);
    gv->setScalarData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->addLabel ("GridVisMode::Columns, interpolated sides", sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::Columns;
    //gv->interpolate_colour_sides = false; // default
    //gv->clr_east_column = mplot::colour::black; // These are defaults but you can change them
    //gv->clr_north_column = mplot::colour::black;
    gv->setScalarData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->addLabel ("GridVisMode::Columns, black sides", sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.05));
    gv->finalize();
    v.addVisualModel (gv);

    offset = { 3 * step * grid.width(), step * grid.width(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::Pixels;
    gv->setScalarData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->addLabel ("GridVisMode::Pixels", sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.05));
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();

    return 0;
}
