/*
 * Showing HSV and HSV1D maps
 */

#include <memory>
#include <format>
#include <vector>
#include <string>

import sm.scale;
import sm.vec;

import mplot.visual;
import mplot.colourbarvisual;
import mplot.hsvwheelvisual;

int main()
{
    std::string title_str = "Hue Saturation Value";
    // Contructor args are width, height, title
    mplot::Visual v(1000, 300, title_str);
    v.setSceneTrans (sm::vec<float,3>{ float{-0.755619}, float{-0.236617}, float{-1.9} });

    sm::scale<float> scale1;
    scale1.compute_scaling (0, 1); // Simply maps 0->1 to 0->1!

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // Hue-variable 1D maps
    std::vector<mplot::ColourMapType> cmap_types;
    cmap_types.push_back (mplot::ColourMapType::HSV1D);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);
    cmap_types.push_back (mplot::ColourMapType::HSV1D);

    mplot::ColourMap<float> cm1;

    // Display HSV wheel
    sm::vec<float, 3> woffset = offset;
    woffset[0] -= 0.5f;
    woffset[1] += 0.25f;
    auto hsvw_vis = std::make_unique<mplot::HSVWheelVisual<float>>(woffset);
    hsvw_vis->set_parent (v.get_id());
    hsvw_vis->setColour (mplot::colour::black);
    hsvw_vis->radius = 0.25f;
    hsvw_vis->tf.fontsize = 0.05f;
    hsvw_vis->labels = {"0", "0.17", "0.33", "0.5", "0.67", "0.83"};
    hsvw_vis->cm.setType (mplot::ColourMapType::HSV);
    hsvw_vis->cm.setHueRotation (-sm::mathconst<float>::pi_over_2);
    hsvw_vis->finalize();
    v.addVisualModel (hsvw_vis);

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

    v.keepOpen();

    return 0;
}
