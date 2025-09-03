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
    std::string title_str = "ColourMaps from William Lenthe";
    mplot::Visual v(1600, 750, title_str);
    v.setSceneTrans (sm::vec<float,3>{ float{-1.88699}, float{0.239456}, float{-3.6} });

    sm::scale<float> scale1;
    scale1.compute_scaling (0, 1); // Simply maps 0->1 to 0->1!

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // 1D maps
    std::vector<mplot::ColourMapType> cmap_types;
    cmap_types.push_back (mplot::ColourMapType::Fire);
    cmap_types.push_back (mplot::ColourMapType::Ocean);
    cmap_types.push_back (mplot::ColourMapType::Ice);
    cmap_types.push_back (mplot::ColourMapType::DivBlueRed);
    cmap_types.push_back (mplot::ColourMapType::CyclicGrey);
    cmap_types.push_back (mplot::ColourMapType::CyclicFour);
    cmap_types.push_back (mplot::ColourMapType::CyclicSix);
    cmap_types.push_back (mplot::ColourMapType::CyclicDivBlueRed);
    cmap_types.push_back (mplot::ColourMapType::Greyscale);
    cmap_types.push_back (mplot::ColourMapType::GreyscaleInv);

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

    /*
     * Maps that encode 2D data
     */

    // HSV and Duochrome maps can be displayed on a Grid
    std::vector<mplot::ColourMapType> cmap_2d_types;
    cmap_2d_types.push_back (mplot::ColourMapType::DiscSixWhite);
    cmap_2d_types.push_back (mplot::ColourMapType::DiscSixBlack);
    cmap_2d_types.push_back (mplot::ColourMapType::DiscFourWhite);
    cmap_2d_types.push_back (mplot::ColourMapType::DiscFourBlack);

    constexpr float pw = 0.03f; // pixel width
    constexpr int N = 20;
    constexpr sm::vec<float, 2> grid_spacing = { pw, pw };
    sm::grid grid(N, N, grid_spacing);
    // Dummy data encodes 2D data
    std::vector<sm::vec<float, 3>> data(grid.n());
    for (int j = 0; j < grid.n(); ++j) {
        data[j] = (grid[j] / (N * pw)).plus_one_dim();
    }

    for (auto cmap_type : cmap_2d_types) {

        auto gv = std::make_unique<mplot::GridVisual<float, int>>(&grid, offset);
        v.bindmodel (gv);
        gv->gridVisMode = mplot::GridVisMode::Triangles;
        gv->setVectorData (&data);
        gv->cm.setType (cmap_type);
        gv->zScale.null_scaling();
        gv->addLabel (mplot::ColourMap<float>::colourMapTypeToStr (cmap_type), {0, -0.1, 0}, mplot::TextFeatures(0.05f));
        gv->twodimensional (true);
        gv->finalize();
        v.addVisualModel (gv);

        offset[0] += 0.8f;
    }

    v.keepOpen();

    return 0;
}
