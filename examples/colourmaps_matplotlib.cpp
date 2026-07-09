/*
 * Many 2D colourbars to show all our different ColourMaps.
 */

#include <memory>
#include <vector>
#include <string>

import sm.scale;
import sm.vec;

import mplot.visual;
import mplot.colourbarvisual;

int main()
{
    std::string title_str = "ColourMaps from Python matplotlib";
    mplot::Visual v(1000, 360, title_str);
    v.setSceneTrans (sm::vec<float,3>{ float{-1.07782}, float{-0.247493}, float{-1.70001} });

    sm::scale<float> scale1;
    scale1.compute_scaling (0, 1); // Simply maps 0->1 to 0->1!

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // 1D maps
    std::vector<mplot::ColourMapType> cmap_types;
    cmap_types.push_back (mplot::ColourMapType::Magma);
    cmap_types.push_back (mplot::ColourMapType::Inferno);
    cmap_types.push_back (mplot::ColourMapType::Plasma);
    cmap_types.push_back (mplot::ColourMapType::Viridis);
    cmap_types.push_back (mplot::ColourMapType::Cividis);
    cmap_types.push_back (mplot::ColourMapType::Twilight);

    mplot::ColourMap<float> cm1(mplot::ColourMapType::Jet);

    // Display 1D colour maps
    int i = 0;
    for (auto cmap_type : cmap_types) {
        ++i;
        cm1.setType (cmap_type);
        auto cbv =  std::make_unique<mplot::ColourBarVisual<float>>(offset);
        cbv->set_parent (v.get_id());
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

    v.keepOpen();

    return 0;
}
