/*
 * An example mplot::Visual scene, containing a HexGrid. This one shows you how to place
 * unicode characters on your coordinate arrows.
 */

#include <memory>
#include <iostream>
#include <vector>
#include <cmath>

import sm.vec;
import sm.hexgrid;

import mplot.visual;
import mplot.visualdatamodel;
import mplot.hexgridvisual;
import mplot.unicode;

int main()
{
    namespace uc = mplot::unicode;

    // Contructor args are width, height, title
    mplot::Visual v(1600, 1000, "mplot::HexGridVisual");
    // You can set a field of view (in degrees)
    v.fov = 15;
    // Make this larger to "scroll in and out of the image" faster
    v.scenetrans_stepsize = 0.5;
    // The coordinate arrows can be hidden
    v.showCoordArrows (true);
    // You can set the background (white, black, or any other colour)
    v.backgroundWhite();
    // You can switch on the "lighting shader" which puts diffuse light into the scene
    v.lightingEffects();
    // Add some text labels to the scene
    v.addLabel ("This is a\nmplot::HexGridVisual\nobject", {0.26f, -0.16f, 0.0f});
    // This function allows you to update the coordinate axes labels (updateCoordLabels ("x", "y", "z"))
    v.updateCoordLabels (uc::toUtf8 (uc::theta), std::string("d") + uc::toUtf8 (uc::beta), "F");

    // Create a hexgrid to show in the scene. Hexes outside the circular boundary will
    // all be discarded.
    sm::hexgrid hg(0.01f, 3.0f, 0.0f);
    hg.set_circular_boundary (0.6f);
    std::cout << "Number of pixels in grid:" << hg.num() << std::endl;


    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(hg.num(), 0.0f);
    for (unsigned int ri=0; ri<hg.num(); ++ri) {
        data[ri] = 0.05f + 0.05f*std::sin(20.0f*hg.d_x[ri]) * std::sin(10.0f*hg.d_y[ri]) ; // Range 0->1
    }

    // Add a HexGridVisual to display the hexgrid within the mplot::Visual scene
    sm::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    auto hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, offset);
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&data);
    hgv->hexVisMode = mplot::HexVisMode::HexInterp; // Or mplot::HexVisMode::Triangles for a smoother surface plot
    hgv->finalize();
    v.addVisualModel (hgv);

    v.keepOpen();

    return 0;
}
