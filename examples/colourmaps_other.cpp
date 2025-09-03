/*
 * Many 2D colourbars to show all our different ColourMaps.
 */

#include <iostream>
#include <vector>
#include <string>

#include <sm/scale>
#include <sm/vec>
#include <sm/grid>

#include <mplot/Visual.h>
#include <mplot/ColourBarVisual.h>
#include <mplot/GridVisual.h>

int main()
{
    // Contructor args are width, height, title
    std::string title_str = "ColourMaps, misc";
    mplot::Visual v(1500, 750, title_str);
    v.setSceneTrans (sm::vec<float,3>{ float{-1.6529}, float{0.232221}, float{-3.6} });

    sm::scale<float> scale1;
    scale1.compute_scaling (0, 1); // Simply maps 0->1 to 0->1!

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // 1D maps
    std::vector<mplot::ColourMapType> cmap_types;
    cmap_types.push_back (mplot::ColourMapType::Petrov);
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monoval);

    mplot::ColourMap<float> cm1(mplot::ColourMapType::Jet);

    // Display 1D colour maps
    int i = 0;
    for (auto cmap_type : cmap_types) {
        ++i;
        cm1.setType (cmap_type);
        auto cbv =  std::make_unique<mplot::ColourBarVisual<float>>(offset);
        v.bindmodel (cbv);
        cbv->orientation = mplot::colourbar_orientation::vertical;
        cbv->tickside = mplot::colourbar_tickside::right_or_below;
        cbv->cm = cm1;
        cbv->scale = scale1;
        cbv->addLabel (mplot::ColourMap<float>::colourMapTypeToStr (cmap_type), {0, -0.1, 0}, mplot::TextFeatures(0.05f));
        cbv->finalize();
        v.addVisualModel (cbv);
        // Update location
        offset[0] += 0.4f;
        if (i % 6 == 0) {
            offset[0] = 0.0f;
            offset[1] -= 1.0f;
        }
    }

    float hue = 0.0f;
    for (int k = 0; k < 6; ++k) {
        ++i;
        cm1.setType (mplot::ColourMapType::HSV1D);
        auto cbv =  std::make_unique<mplot::ColourBarVisual<float>>(offset);
        v.bindmodel (cbv);
        cbv->orientation = mplot::colourbar_orientation::vertical;
        cbv->tickside = mplot::colourbar_tickside::right_or_below;
        cbv->cm = cm1;
        // Set the 'hue' angle (range 0 to 1)
        cbv->cm.setHue (hue);
        cbv->scale = scale1;
        cbv->addLabel ("HSV1D " + std::to_string(static_cast<int>(hue * 360)), {0, -0.1, 0}, mplot::TextFeatures(0.05f));
        cbv->finalize();
        v.addVisualModel (cbv);
        // Increment hue
        hue += 0.2f;
        // Update location
        offset[0] += 0.4f;
        if (i % 6 == 0) {
            offset[0] = 0.0f;
            offset[1] -= 1.0f;
        }
    }

    /*
     * Maps that encode 2D data
     */

    // HSV and Duochrome maps can be displayed on a Grid
    std::vector<mplot::ColourMapType> cmap_2d_types;
    cmap_2d_types.push_back (mplot::ColourMapType::HSV);
    cmap_2d_types.push_back (mplot::ColourMapType::Duochrome);

    constexpr float pw = 0.03f; // pixel width
    constexpr int N = 20;
    constexpr sm::vec<float, 2> grid_spacing = { pw, pw };
    sm::grid grid(N, N, grid_spacing);
    // Dummy data encodes 2D data
    std::vector<sm::vec<float, 3>> data(grid.n());
    for (int j = 0; j < grid.n(); ++j) {
        data[j] = (grid[j] / (N * pw)).plus_one_dim();
    }

    auto gv = std::make_unique<mplot::GridVisual<float, int>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = mplot::GridVisMode::Triangles;
    gv->setVectorData (&data);
    gv->cm.setType (cmap_2d_types[0]);
    gv->zScale.null_scaling();
    gv->addLabel (mplot::ColourMap<float>::colourMapTypeToStr (cmap_2d_types[0]), {0, -0.1, 0}, mplot::TextFeatures(0.05f));
    gv->twodimensional (true);
    gv->finalize();
    v.addVisualModel (gv);

    offset[0] += 0.8f;

    gv = std::make_unique<mplot::GridVisual<float, int>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = mplot::GridVisMode::Triangles;
    gv->setVectorData (&data);
    gv->cm.setType (cmap_2d_types[1]);
    gv->zScale.null_scaling();
    gv->addLabel ("Duochrome red-green", {0, -0.1, 0}, mplot::TextFeatures(0.05f));
    gv->twodimensional (true);
    gv->finalize();
    v.addVisualModel (gv);

    offset[0] += 0.8f;

    gv = std::make_unique<mplot::GridVisual<float, int>>(&grid, offset);
    v.bindmodel (gv);
    gv->gridVisMode = mplot::GridVisMode::Triangles;
    gv->setVectorData (&data);
    gv->cm.setType (cmap_2d_types[1]);
    gv->cm.setHueRB();
    gv->zScale.null_scaling();
    gv->addLabel ("Duochrome red-blue", {0, -0.1, 0}, mplot::TextFeatures(0.05f));
    gv->twodimensional (true);
    gv->finalize();
    v.addVisualModel (gv);

    v.keepOpen();

    return 0;
}
