#include <iostream>

// include HexGrid with the optional load() and save() methods
#define HEXGRID_COMPILE_LOAD_AND_SAVE 1
#include <sm/hexgrid>

#include <sm/vec>

#include <mplot/ReadCurves.h>
#include <mplot/tools.h>
#include <mplot/ColourMap.h>
#include <mplot/Visual.h>
#include <mplot/HexGridVisual.h>

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

        sm::hexgrid hg(0.01, 3, 0);
        hg.setBoundary (r.getCorticalPath());

        std::cout << hg.extent() << std::endl;

        hexnum = hg.num();
        std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
        std::cout << "Last vector index:" << hg.lastVectorIndex() << std::endl;

        hg.save("../trialhexgrid.h5");

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading trial.svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << mplot::tools::getPwd() << std::endl;
        rtn = -1;
    }
    std::cout << "Generated " << mplot::tools::timeNow() << std::endl;
    // Now read it back
    try {
        sm::hexgrid hg("../trialhexgrid.h5");

        std::cout << "Read " << mplot::tools::timeNow() << std::endl;

        // Make sure read-in grid has same number of hexes as the generated one.
        if (hexnum != hg.num()) { rtn = -1; }

        // Create a hexgrid Visual
        mplot::Visual v(1600, 1000, "hexgrid");
        v.lightingEffects();
        sm::vec<float, 3> offset = { 0.0f, -0.0f, 0.0f };
        auto hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, offset);
        v.bindmodel (hgv);
        // Set up data for the HexGridVisual and colour hexes according to their state as being boundary/inside/domain, etc
        std::vector<float> colours (hg.num(), 0.0f);
        static constexpr float cl_boundary_and_in = 0.9f;
        static constexpr float cl_bndryonly = 0.8f;
        static constexpr float cl_domain = 0.5f;
        static constexpr float cl_inside = 0.15f;
        if (hg.d_flags.size() < hg.num()) { throw std::runtime_error ("d_flags not present"); }
        // Note, HexGridVisual uses d_x and d_y vectors, so set colours according to d_flags vector
        for (unsigned int i = 0; i < hg.num(); ++i) {
            if (hg.d_flags[i] & HEX_IS_BOUNDARY ? true : false
                && hg.d_flags[i] & HEX_INSIDE_BOUNDARY ? true : false) {
                // red is boundary hex AND inside boundary
                colours[i] = cl_boundary_and_in;
            } else if (hg.d_flags[i] & HEX_IS_BOUNDARY ? true : false) {
                // orange is boundary ONLY
                colours[i] = cl_bndryonly;
            } else if (hg.d_flags[i] & HEX_INSIDE_BOUNDARY ? true : false) {
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

        while (v.readyToFinish() == false) {
            glfwWaitEventsTimeout (0.018);
            v.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading trial.svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << mplot::tools::getPwd() << std::endl;
        rtn = -1;
    }
    return rtn;
}
