/*
 * Showing hue-variable 1D colour maps.
 */

#include <memory>
#include <format>
#include <array>
#include <vector>
#include <string>

import sm.scale;
import sm.vec;

import mplot.visual;
import mplot.colourbarvisual;

int main()
{
    // Contructor args are width, height, title
    std::string title_str = "ColourMaps";
    mplot::Visual v(1000, 1200, title_str);
    v.setSceneTrans (sm::vec<float,3>{ float{-1.11157}, float{0.762484}, float{-5.7} });

    sm::scale<float> scale1;
    scale1.compute_scaling (0, 1); // Simply maps 0->1 to 0->1!

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // Hue-variable 1D maps
    std::vector<mplot::ColourMapType> cmap_types;
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monoval);
    cmap_types.push_back (mplot::ColourMapType::Monoval);
    cmap_types.push_back (mplot::ColourMapType::Monoval);
    cmap_types.push_back (mplot::ColourMapType::Monoval);
    cmap_types.push_back (mplot::ColourMapType::Monoval);
    cmap_types.push_back (mplot::ColourMapType::Monoval);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);

    mplot::ColourMap<float> cm1;

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
        float hue = offset[1]*(-0.05555555555f) + offset[0] / 2.4f; // Use x value to set hue
        cbv->cm.setHue (hue);
        cbv->scale = scale1;
        // mplot::ColourMap<float>::colourMapTypeToStr (cmap_type)
        cbv->addLabel (std::format("hue={:.2f}", hue), {0, -0.1, 0}, mplot::TextFeatures(0.05f));
        cbv->finalize();
        v.addVisualModel (cbv);
        // Update location
        offset[0] += 0.4f;
        if (i % 6 == 0) {
            offset[0] = 0.0f;
            offset[1] -= 1.0f;
        }
    }

    cmap_types.clear();
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monochrome);
    cmap_types.push_back (mplot::ColourMapType::Monoval);
    cmap_types.push_back (mplot::ColourMapType::Monoval);

    std::array<float, 3> purple = sm::vec<int, 3>({0x68, 0x31, 0x92}).as_float()/255.0f;
    std::array<float, 3> orange = sm::vec<int, 3>({0xdf, 0x5e, 0x26}).as_float()/255.0f;
    std::array<float, 3> blue = sm::vec<int, 3>({0x2a, 0x37, 0x91}).as_float()/255.0f;
    std::array<float, 3> green = sm::vec<int, 3>({0x5b, 0x89, 0x3d}).as_float()/255.0f;
    //std::array<float, 3> purple2 = sm::vec<int, 3>({0xa4, 0x84, 0xbc}).as_float()/255.0f;
    std::array<float, 3> orange2 = sm::vec<int, 3>({0xee, 0x9f, 0x7d}).as_float()/255.0f;

    std::vector<std::array<float, 3>> clrs;
    clrs.push_back (purple);
    clrs.push_back (orange);
    clrs.push_back (green);
    clrs.push_back (blue);
    clrs.push_back (orange);
    clrs.push_back (orange2);

    int j = 0;
    for (auto cmap_type : cmap_types) {
        ++i;
        cm1.setType (cmap_type);
        cm1.setRGB (clrs[j++]);
        auto cbv =  std::make_unique<mplot::ColourBarVisual<float>>(offset);
        cbv->set_parent (v.get_id());
        cbv->orientation = mplot::colourbar_orientation::vertical;
        cbv->tickside = mplot::colourbar_tickside::right_or_below;
        cbv->cm = cm1;
        cbv->scale = scale1;
        // mplot::ColourMap<float>::colourMapTypeToStr (cmap_type)
        cbv->addLabel (std::format("hue={:.2f}", cbv->cm.getHue()), {0, -0.1, 0}, mplot::TextFeatures(0.05f));
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
