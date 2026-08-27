/*
 * An example mplot::Visual scene, containing a HexGrid.
 */

#include <memory>
#include <iostream>
#include <vector>
#include <cmath>

import sm.vec;
import sm.hexgrid;

import mplot.visual;
import mplot.hexgridvisual;

int main()
{
    mplot::Visual v(1600, 1000, "mplot::Visual");
    // You can set a field of view (in degrees)
    v.fov = 15;
    // Should the scene be 'locked' so that movements and rotations are prevented?
    v.sceneLocked (false);
    // Various methods to set the 'scene translation'. Try pressing 'z' in the app window to see what the current sceneTrans is
    v.setSceneTransXY (0.0f, 0.0f);
    v.setSceneTransZ (-6.0f);
    v.setSceneTrans (sm::vec<float, 3>{0.0f, 0.0f, -6.0f});
    // Make this larger to "scroll in and out of the image" faster
    v.scenetrans_stepsize = 0.5;
    // The coordinate arrows can be hidden
    v.showCoordArrows (true);
    // The title can be shown, or hidden (default)
    v.showTitle (true);
    // The coord arrows can be displayed within the scene (rather than in, say, the corner)
    v.coordArrowsInScene (false);
    // You can set the background (white, black, or any other colour)
    v.backgroundWhite();
    // You can switch on the "lighting shader" which puts diffuse light into the scene
    v.lightingEffects();
    // Add some text labels to the scene
    v.addLabel ("Each object is derived from mplot::VisualModel", {0.005f, -0.02f, 0.0f});
    v.addLabel ("This is a mplot::CoordArrows object", {0.03f, -0.23f, 0.0f});
    v.addLabel ("This is a\nmplot::HexGridVisual\nobject", {0.26f, -0.16f, 0.0f});

    // Create a hexgrid to show in the scene
    sm::hexgrid<float> hg(0.01, 3, 0);
    hg.set_circular_boundary (0.3);
    std::cout << "Number of hexes in grid:" << hg.num() << std::endl;

    // Make some dummy data (a sine wave) to make an interesting surface
    std::vector<float> data(hg.num(), 0.0);
    for (unsigned int hi = 0; hi < hg.num(); ++hi) {
        data[hi] = 0.05f + 0.05f * std::sin (10.0f * hg.d_x[hi]); // Range 0->1
    }

    // Add a HexGridVisual to display the hexgrid within the mplot::Visual scene
    sm::vec<float, 3> offset = { 0.0, -0.05, 0.0 };
    auto hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, offset);
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&data);
    hgv->finalize();
    v.addVisualModel (hgv);

    v.keepOpen();

    v.savegltf("./visual.gltf");

    return 0;
}
