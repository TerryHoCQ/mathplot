/*
 * Showing the Crameri colourmaps
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
    std::string title_str = "ColourMaps from Fabio Crameri";
    mplot::Visual v(1000, 1400, title_str);
    v.setSceneTrans (sm::vec<float,3>{ float{-1.52137}, float{1.74665}, float{-9.60001} });

    sm::scale<float> scale1;
    scale1.compute_scaling (0, 1); // Simply maps 0->1 to 0->1!

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // 1D maps
    std::vector<mplot::ColourMapType> cmap_types;
    cmap_types.push_back (mplot::ColourMapType::Devon);
    cmap_types.push_back (mplot::ColourMapType::NaviaW);
    cmap_types.push_back (mplot::ColourMapType::BrocO);
    cmap_types.push_back (mplot::ColourMapType::Acton);
    cmap_types.push_back (mplot::ColourMapType::Batlow);
    cmap_types.push_back (mplot::ColourMapType::Berlin);
    cmap_types.push_back (mplot::ColourMapType::Tofino);
    cmap_types.push_back (mplot::ColourMapType::Broc);
    cmap_types.push_back (mplot::ColourMapType::CorkO);
    cmap_types.push_back (mplot::ColourMapType::Lapaz);
    cmap_types.push_back (mplot::ColourMapType::BamO);
    cmap_types.push_back (mplot::ColourMapType::Vanimo);
    cmap_types.push_back (mplot::ColourMapType::Lajolla);
    cmap_types.push_back (mplot::ColourMapType::Lisbon);
    cmap_types.push_back (mplot::ColourMapType::GrayC);
    cmap_types.push_back (mplot::ColourMapType::Roma);
    cmap_types.push_back (mplot::ColourMapType::Vik);
    cmap_types.push_back (mplot::ColourMapType::Navia);
    cmap_types.push_back (mplot::ColourMapType::Bilbao);
    cmap_types.push_back (mplot::ColourMapType::Turku);
    cmap_types.push_back (mplot::ColourMapType::Lipari);
    cmap_types.push_back (mplot::ColourMapType::VikO);
    cmap_types.push_back (mplot::ColourMapType::BatlowK);
    cmap_types.push_back (mplot::ColourMapType::Oslo);
    cmap_types.push_back (mplot::ColourMapType::Oleron);
    cmap_types.push_back (mplot::ColourMapType::Davos);
    cmap_types.push_back (mplot::ColourMapType::Fes);
    cmap_types.push_back (mplot::ColourMapType::Managua);
    cmap_types.push_back (mplot::ColourMapType::Glasgow);
    cmap_types.push_back (mplot::ColourMapType::Tokyo);
    cmap_types.push_back (mplot::ColourMapType::Bukavu);
    cmap_types.push_back (mplot::ColourMapType::Bamako);
    cmap_types.push_back (mplot::ColourMapType::BatlowW);
    cmap_types.push_back (mplot::ColourMapType::Nuuk);
    cmap_types.push_back (mplot::ColourMapType::Cork);
    cmap_types.push_back (mplot::ColourMapType::Hawaii);
    cmap_types.push_back (mplot::ColourMapType::Bam);
    cmap_types.push_back (mplot::ColourMapType::Imola);
    cmap_types.push_back (mplot::ColourMapType::RomaO);
    cmap_types.push_back (mplot::ColourMapType::Buda);

    mplot::ColourMap<float> cm1(mplot::ColourMapType::Acton);

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
        if (i % 8 == 0) {
            offset[0] = 0.0f;
            offset[1] -= 1.0f;
        }
    }

    v.keepOpen();

    return 0;
}
