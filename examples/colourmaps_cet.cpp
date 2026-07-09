/*
 * Showing the CET colourmaps
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
    // Contructor args are width, height, title
    std::string title_str = "ColourMaps from colorcet.com";
    mplot::Visual v(1400, 1250, title_str);
    v.setSceneTrans (sm::vec<float,3>{ float{-2.45365}, float{1.74889}, float{-9} });

    sm::scale<float> scale1;
    scale1.compute_scaling (0, 1); // Simply maps 0->1 to 0->1!

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // 1D maps
    std::vector<mplot::ColourMapType> cmap_types;
    cmap_types.push_back (mplot::ColourMapType::Jet);
    cmap_types.push_back (mplot::ColourMapType::Rainbow);
    cmap_types.push_back (mplot::ColourMapType::CET_L02);
    cmap_types.push_back (mplot::ColourMapType::CET_L13);
    cmap_types.push_back (mplot::ColourMapType::CET_C4);
    cmap_types.push_back (mplot::ColourMapType::CET_D04);
    cmap_types.push_back (mplot::ColourMapType::CET_L12);
    cmap_types.push_back (mplot::ColourMapType::CET_C1s);
    cmap_types.push_back (mplot::ColourMapType::CET_L01);
    cmap_types.push_back (mplot::ColourMapType::CET_C5);
    cmap_types.push_back (mplot::ColourMapType::CET_D11);
    cmap_types.push_back (mplot::ColourMapType::CET_L04);
    cmap_types.push_back (mplot::ColourMapType::CET_CBL2);
    cmap_types.push_back (mplot::ColourMapType::CET_C4s);
    cmap_types.push_back (mplot::ColourMapType::CET_L15);
    cmap_types.push_back (mplot::ColourMapType::CET_L20);
    cmap_types.push_back (mplot::ColourMapType::CET_CBD1);
    cmap_types.push_back (mplot::ColourMapType::CET_D06);
    cmap_types.push_back (mplot::ColourMapType::CET_I3);
    cmap_types.push_back (mplot::ColourMapType::CET_D01A);
    cmap_types.push_back (mplot::ColourMapType::CET_L16);
    cmap_types.push_back (mplot::ColourMapType::CET_L06);
    cmap_types.push_back (mplot::ColourMapType::CET_C2s);
    cmap_types.push_back (mplot::ColourMapType::CET_I1);
    cmap_types.push_back (mplot::ColourMapType::CET_C7s);
    cmap_types.push_back (mplot::ColourMapType::CET_I2);
    cmap_types.push_back (mplot::ColourMapType::CET_C6s);
    cmap_types.push_back (mplot::ColourMapType::CET_C6);
    cmap_types.push_back (mplot::ColourMapType::CET_L05);
    cmap_types.push_back (mplot::ColourMapType::CET_D08);
    cmap_types.push_back (mplot::ColourMapType::CET_L03);
    cmap_types.push_back (mplot::ColourMapType::CET_L14);
    cmap_types.push_back (mplot::ColourMapType::CET_C2);
    cmap_types.push_back (mplot::ColourMapType::CET_R3);
    cmap_types.push_back (mplot::ColourMapType::CET_D01);
    cmap_types.push_back (mplot::ColourMapType::CET_C1);
    cmap_types.push_back (mplot::ColourMapType::CET_D02);
    cmap_types.push_back (mplot::ColourMapType::CET_CBC1);
    cmap_types.push_back (mplot::ColourMapType::CET_D09);
    cmap_types.push_back (mplot::ColourMapType::CET_L10);
    cmap_types.push_back (mplot::ColourMapType::CET_R1);
    cmap_types.push_back (mplot::ColourMapType::CET_C3);
    cmap_types.push_back (mplot::ColourMapType::CET_CBL1);
    cmap_types.push_back (mplot::ColourMapType::CET_C3s);
    cmap_types.push_back (mplot::ColourMapType::CET_C5s);
    cmap_types.push_back (mplot::ColourMapType::CET_L08);
    cmap_types.push_back (mplot::ColourMapType::CET_R4);
    cmap_types.push_back (mplot::ColourMapType::CET_R2);
    cmap_types.push_back (mplot::ColourMapType::CET_L11);
    cmap_types.push_back (mplot::ColourMapType::CET_D10);
    cmap_types.push_back (mplot::ColourMapType::CET_D07);
    cmap_types.push_back (mplot::ColourMapType::CET_L17);
    cmap_types.push_back (mplot::ColourMapType::CET_D12);
    cmap_types.push_back (mplot::ColourMapType::CET_CBC2);
    cmap_types.push_back (mplot::ColourMapType::CET_D13);
    cmap_types.push_back (mplot::ColourMapType::CET_D03);
    cmap_types.push_back (mplot::ColourMapType::CET_C7);
    cmap_types.push_back (mplot::ColourMapType::CET_L07);
    cmap_types.push_back (mplot::ColourMapType::CET_L09);
    cmap_types.push_back (mplot::ColourMapType::CET_L18);
    cmap_types.push_back (mplot::ColourMapType::CET_L19);

    mplot::ColourMap<float> cm1(mplot::ColourMapType::Plasma);

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
        if (i % 13 == 0) {
            offset[0] = 0.0f;
            offset[1] -= 1.0f;
        }
    }

    v.keepOpen();

    return 0;
}
