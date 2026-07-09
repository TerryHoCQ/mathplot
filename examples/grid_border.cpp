/*
 * An example mplot::Visual scene, containing a Gridv, and using GridvVisual
 */

#include <memory>
#include <iostream>
#include <string>
#include <cstdlib>

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
    constexpr unsigned int N_pix_w = 25;
    constexpr unsigned int N_pix_h = 8;
    constexpr sm::vec<float, 2> grid_spacing = {0.2f, 0.2f};

    // The simplest declaration of Grid is:
    //   sm::grid g(size_t n_x, size_t n_y);
    // grid_spacing, grid_zero, use of memory, wrapping and ordering are all possible arguments to
    // the constructor.
    sm::grid<unsigned int, float> grid(N_pix_w, N_pix_h, grid_spacing);

    std::cout << "Number of pixels in grid:" << grid.n() << std::endl;

    // Make some dummy data (a sine wave) to make an interesting surface
    sm::vvec<sm::vec<float, 3>> data(grid.n(), sm::vec<float, 3>({0.0f, 0.0f, 0.0f}));
    for (unsigned int ri=0; ri<grid.n(); ++ri) {
        // auto coord = grid[ri];
        data[ri][0] =  static_cast<double>(std::rand()) / RAND_MAX ; // Range 0->1
    }

    float step = 0.64f;
    // Add a GridVisual to display the grid within the mplot::Visual scene
    sm::vec<float, 3> offset = { -step * grid.width(), step * grid.height(), 0.0f };

    // 1) visualizing vector with GridVisMode = RectInterp
    auto gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->addLabel ("1) Base GridVisMode::RectInterp", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 2) same as 1 with zScale set to 0
    offset = { step * grid.width(), step * grid.height(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->zScale.null_scaling();
    gv->addLabel ("2) 1 + no zScale", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 3) same as 2 with border ON and border colour set to cyan
    offset = { 3 * step * grid.width(), step * grid.height(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->zScale.null_scaling();
    gv->showborder (true);
    gv->border_thickness = 0.25f;
    gv->border_colour = mplot::colour::cyan;
    gv->border_tubular (false);
    gv->addLabel ("3) 2 + flat border", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 4) same as 2 with border ON and border colour set to cyan
    offset = { 5 * step * grid.width(), step * grid.height(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->showborder (true);
    gv->border_thickness = 0.25f;
    gv->border_colour = mplot::colour::cyan;
    gv->border_tubular (false);
    gv->addLabel ("4) 1 + flat border", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 5) same as 2 + grid
    offset = { step * grid.width(), -step * grid.height(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->zScale.null_scaling();
    gv->showgrid (true);
    gv->grid_colour = mplot::colour::black;
    gv->grid_thickness = 0.1f;
    gv->addLabel ("5) 2 + grid ", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 6) show both border and grid
    offset = { 3 * step * grid.width(), -step * grid.height(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->zScale.null_scaling();
    gv->showgrid (true);
    gv->grid_colour = mplot::colour::black;
    gv->grid_thickness = 0.1f;
    gv->showborder (true);
    gv->border_thickness = 0.25f;
    gv->border_colour = mplot::colour::magenta;
    gv->border_tubular (true); // it's true by default
    gv->addLabel ("6) 5 + tubular border ", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 7) show how to use the selected pixel option
    offset = { step * grid.width(), -3 * step * grid.height(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->zScale.null_scaling();

    gv->showselectedpixborder (true);
    gv->selected_pix[6] = mplot::colour::forestgreen;
    gv->selected_pix[0] = mplot::colour::yellow3;
    gv->selected_pix[9] = {1,0.2431372549,0.5882352941};
    gv->selected_pix[10] = mplot::colour::skyblue;
    gv->selected_pix[124] = mplot::colour::tomato2;
    gv->selected_pix[125] = mplot::colour::gray55;
    gv->selected_pix[49] = mplot::colour::red2;
    gv->selected_pix[90] = mplot::colour::tan1;
    gv->selected_pix[99] = mplot::colour::gold;

    gv->grid_thickness = 0.1f;

    gv->addLabel ("7) 2 + selected pixel borders", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // 8) show how to use the selected pixel option with grid
    offset = { 3 * step * grid.width(), -3 * step * grid.height(), 0.0f };
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setVectorData (&data);
    gv->cm.setType (mplot::ColourMapType::Twilight);
    gv->zScale.null_scaling();

    gv->showgrid (true);
    gv->grid_colour = mplot::colour::black;
    gv->grid_thickness = 0.05f;

    gv->showselectedpixborder (true);
    gv->selected_pix_thickness = 0.1f;

    gv->selected_pix[6] = mplot::colour::forestgreen;
    gv->selected_pix[0] = mplot::colour::yellow3;
    gv->selected_pix[9] = {1,0.2431372549,0.5882352941};
    gv->selected_pix[10] = mplot::colour::skyblue;
    gv->selected_pix[124] = mplot::colour::tomato2;
    gv->selected_pix[125] = mplot::colour::gray55;
    gv->selected_pix[49] = mplot::colour::red2;
    gv->selected_pix[90] = mplot::colour::tan1;
    gv->selected_pix[99] = mplot::colour::gold;

    gv->addLabel ("8) 7 + grid", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();

    return 0;
}
