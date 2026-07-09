/*
 * A 3D surface with an associated 2D colourbar.
 */

#include <memory>
#include <iostream>
#include <vector>
#include <cmath>

import sm.vec;
import sm.vvec;
import sm.hexgrid;

import mplot.visual;
//import mplot.visualdatamodel;
import mplot.hexgridvisual;
import mplot.colourbarvisual;

int main()
{
    // Choose a colourmap.
    //
    // Options include:
    // Jet, Plasma, Inferno, Twilight, Rainbow, Viridis, Cividis, Greyscale, MonochromeRed, MonovalRed
    //
    // See mplot/ColourMap.h for the full list in the definition of ColourMapType
    constexpr mplot::ColourMapType colour_map_type = mplot::ColourMapType::Inferno;

    // Contructor args are width, height, title, coordinate arrows offset, cooridnate
    // arrows lengths, coord arrow thickness, coord arrow font size (0 means no labels)
    std::string title_str = "ColourBar (" + mplot::ColourMap<float>::colourMapTypeToStr (colour_map_type) + ")";
    mplot::Visual v(1200, 1000, title_str);
    // Position with some scene trans setup code (try Ctrl-z in the program and see stdout):
    v.setSceneTrans (sm::vec<float,3>({-0.140266f, 0.237435f, -3.5f}));

    // A hexgrid to show in the scene.
    sm::hexgrid hg(0.01f, 3.0f, 0.0f);
    hg.set_circular_boundary (0.6f);
    std::cout << "Number of pixels in grid:" << hg.num() << std::endl;

    // Make some data for the surface
    sm::vvec<float> data(hg.num(), 0.0f);
    for (unsigned int ri=0; ri<hg.num(); ++ri) {
        data[ri] = 0.00001f + 0.05f + 0.05f*std::sin(20.0f*hg.d_x[ri]) * std::sin(10.0f*hg.d_y[ri]) ; // Range 0->1
    }

    // Add a HexGridVisual to display the hexgrid within the mplot::Visual scene
    sm::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    auto hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, offset);
    hgv->set_parent (v.get_id());
    hgv->cm.setType (colour_map_type); // This is how we set the colour map type in HexGridVisual
    hgv->setScalarData (&data);
    hgv->hexVisMode = mplot::HexVisMode::Triangles;
    hgv->finalize();
    auto hgvp = v.addVisualModel (hgv);

    // Add the colour bar
    offset = {0.8f, -0.3f, 0.0f};
    auto cbv = std::make_unique<mplot::ColourBarVisual<float>>(offset);
    cbv->set_parent (v.get_id());
    cbv->orientation = mplot::colourbar_orientation::vertical;
    cbv->tickside = mplot::colourbar_tickside::right_or_below;
    // Copy colourmap and scale to colourbar visual
    cbv->cm = hgvp->cm;
    cbv->scale = hgvp->colourScale;
    // Now build it
    cbv->finalize();
    v.addVisualModel (cbv);

    // Add a horizontal colourbar, too
    offset = {-0.3f, -1.0f, 0.0f};
    cbv = std::make_unique<mplot::ColourBarVisual<float>>(offset);
    cbv->set_parent (v.get_id());
    cbv->orientation = mplot::colourbar_orientation::horizontal;
    cbv->tickside = mplot::colourbar_tickside::left_or_above;
    cbv->cm = hgvp->cm;
    cbv->scale = hgvp->colourScale;
    std::string lbl = "ColourMapType: " + mplot::ColourMap<float>::colourMapTypeToStr (colour_map_type);
    cbv->addLabel (lbl, sm::vec<float>{ 0.0f, -0.08f, 0.0f }, mplot::TextFeatures(0.05f));
    cbv->finalize();
    v.addVisualModel (cbv);

    v.keepOpen();

    return 0;
}
