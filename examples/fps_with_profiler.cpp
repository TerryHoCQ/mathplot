/*
 * An example mplot::Visual scene, containing a HexGrid.
 *
 * Using mplot::fps::profiler for the FPS profile
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <memory>

import sm.vec;
import sm.hexgrid;

import mplot.visual;
import mplot.visualdatamodel;
import mplot.visualtextmodel;
import mplot.hexgridvisual;

import mplot.fps.profiler;

int main()
{
    mplot::Visual v(1600, 1000, "mplot::Visual");
    v.fov = 15.0f;
    v.zFar = 200.0f;
    v.lightingEffects();
    mplot::VisualTextModel<>* fps_tm;
    v.addLabel ("0 FPS", {0.13f, -0.23f, 0.0f}, fps_tm); // With fps_tm can update the VisualTextModel with fps_tm->setupText("new text")

    // Create a hexgrid to show in the scene
    constexpr float hex_to_hex = 0.02f;

    sm::hexgrid hg(hex_to_hex, 15.0f, 0.0f);
    hg.set_elliptical_boundary (4.0f, 4.0f);
    std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
    std::stringstream sss;
    sss << "Surface evaluated at " << hg.num() << " coordinates";
    v.addLabel (sss.str(), {0.0f, 0.0f, 0.0f});

    // Make some dummy data (a radially symmetric Bessel fn) to make an interesting surface
    std::vector<float> data(hg.num(), 0.0f);
    std::vector<float> r(hg.num(), 0.0f);
    float k = 1.0f;
    for (unsigned int hi = 0; hi < hg.num(); ++hi) {
        // x/y: hg.d_x[hi] hg.d_y[hi]
        r[hi] = std::sqrt (hg.d_x[hi] * hg.d_x[hi] + hg.d_y[hi] * hg.d_y[hi]);
        data[hi] = std::sin (k * r[hi]) / k * r[hi];
    }

    // Add a HexGridVisual to display the HexGrid within the mplot::Visual scene
    sm::vec<float, 3> offset = { 0.0f, -0.05f, 0.0f };
    auto hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, offset);
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&data);
    hgv->hexVisMode = mplot::HexVisMode::Triangles;
    hgv->finalize();
    auto hgvp = v.addVisualModel (hgv);

    unsigned int fcount = 0u;

    // Our profiler object
    mplot::fps::profiler fps_profiler;

    while (v.readyToFinish() == false) {

        v.poll();

        fps_profiler.at_begin (1000);

        if (k > 8.0f) { k = 1.0f; }

#pragma omp parallel for shared(r,k,data)
        for (unsigned int hi = 0; hi < hg.num(); ++hi) {
            data[hi] = std::sin (k * r[hi]) / k * r[hi];
        }
        if (v.validVisualModel (hgvp) != nullptr) { // Test hgvp is still valid
            hgvp->updateData (&data);
        }
        k += 0.02f;

        if (fcount == 500) { // Update FPS text
            fps_tm->setupText (fps_profiler.fps_txt);
            fcount = 0;
            v.waitevents (0.00001);
        }

        v.render();
        fcount++;

        fps_profiler.at_end();
    }

    return 0;
}
