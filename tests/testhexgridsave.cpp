#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

import sm.vec;
import sm.hexgrid;
import sm.hexgrid.hdf; // load and save code

import mplot.tools;
import mplot.colourmap;
import mplot.readcurves;
import mplot.visual;
import mplot.hexgridvisual;

int main()
{
    int rtn = 0;
    unsigned int hexnum = 0;

    std::cout << "Start " << mplot::tools::timeNow() << std::endl;
    // Create and then write a hexgrid
    try {
        std::string pwd = mplot::tools::getPwd();
        std::string curvepath = "../../tests/trial.svg";

        mplot::ReadCurves r(curvepath);

        sm::hexgrid<float> hg(0.01, 3, 0);
        hg.set_boundary (r.getCorticalPath());

        std::cout << hg.extent() << std::endl;

        hexnum = hg.num();
        std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
        std::cout << "Last vector index:" << hg.last_vector_index() << std::endl;

        sm::hexgrid_save (hg, "../trialhexgrid.h5");

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading trial.svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << mplot::tools::getPwd() << std::endl;
        rtn = -1;
    }
    std::cout << "Generated " << mplot::tools::timeNow() << std::endl;
    // Now read it back
    try {
        sm::hexgrid<float> hg;
        sm::hexgrid_load (hg, "../trialhexgrid.h5");

        std::cout << "Read " << mplot::tools::timeNow() << std::endl;

        // Make sure read-in grid has same number of hexes as the generated one.
        if (hexnum != hg.num()) { rtn = -1; }

        // Create a hexgrid Visual
        mplot::Visual v(1600, 1000, "hexgrid");
        v.lightingEffects();
        sm::vec<float, 3> offset = { 0.0f, -0.0f, 0.0f };
        auto hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, offset);
        hgv->set_parent (v.get_id());
        // Set up data for the HexGridVisual and colour hexes according to their state as being boundary/inside/domain, etc
        std::vector<float> colours (hg.num(), 0.0f);
        static constexpr float cl_boundary_and_in = 0.9f;
        static constexpr float cl_bndryonly = 0.8f;
        static constexpr float cl_domain = 0.5f;
        static constexpr float cl_inside = 0.15f;
        if (hg.d_flags.size() < hg.num()) { throw std::runtime_error ("d_flags not present"); }
        // Note, HexGridVisual uses d_x and d_y vectors, so set colours according to d_flags vector
        for (unsigned int i = 0; i < hg.num(); ++i) {
            if (hg.d_flags[i] & sm::HEX_IS_BOUNDARY ? true : false
                && hg.d_flags[i] & sm::HEX_INSIDE_BOUNDARY ? true : false) {
                // red is boundary hex AND inside boundary
                colours[i] = cl_boundary_and_in;
            } else if (hg.d_flags[i] & sm::HEX_IS_BOUNDARY ? true : false) {
                // orange is boundary ONLY
                colours[i] = cl_bndryonly;
            } else if (hg.d_flags[i] & sm::HEX_INSIDE_BOUNDARY ? true : false) {
                // Inside boundary -  blue
                colours[i] = cl_inside;
            } else {
                // The domain - greenish
                colours[i] = cl_domain;
            }
        }
        hgv->cm.setType (mplot::ColourMapType::Jet);
        hgv->zScale.null_scaling(); // makes the output flat in z direction, but you still get the colours
        hgv->setScalarData (&colours);
        hgv->hexVisMode = mplot::HexVisMode::HexInterp; // Or mplot::HexVisMode::Triangles for a smoother surface plot
        hgv->finalize();
        v.addVisualModel (hgv);

        // Would be nice to:
        // Draw small hex at boundary centroid.
        // red hex at zero
        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading trial.svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << mplot::tools::getPwd() << std::endl;
        rtn = -1;
    }
    return rtn;
}
