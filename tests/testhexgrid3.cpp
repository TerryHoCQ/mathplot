#include <utility>
#include <iostream>

#include <sm/vec>
#include <sm/hexgrid>

#include <mplot/tools.h>
#include <mplot/ColourMap.h>
#include <mplot/ReadCurves.h>
#include <mplot/Visual.h>
#include <mplot/HexGridVisual.h>

int main()
{
    int rtn = 0;
    try {
        std::string pwd = mplot::tools::getPwd();
        std::string curvepath = "../../tests/trialmod.svg";
        mplot::ReadCurves r(curvepath);

        sm::hexgrid hg(0.02, 7, 0);
        hg.setBoundary (r.getCorticalPath());

        std::cout << hg.extent() << std::endl;
        std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
        std::cout << "Last vector index:" << hg.lastVectorIndex() << std::endl;

        if (hg.num() != 2088 && hg.num() != 2087) {
            std::cerr << "hg num (" << hg.num() << ") not equal to 2087/2088..." << std::endl;
            rtn = -1;
        }

        // Create a HexGrid
        mplot::Visual v(1600, 1000, "HexGrid");
        v.lightingEffects();
        sm::vec<float, 3> offset = { 0.0f, -0.0f, 0.0f };
        auto hgv = std::make_unique<mplot::HexGridVisual<float>> (&hg, offset);
        v.bindmodel (hgv);
        // Set up data for the HexGridVisual and colour hexes according to their state as being boundary/inside/domain, etc
        std::vector<float> colours (hg.num(), 0.0f);
        static constexpr float cl_boundary_and_in = 0.9f;
        static constexpr float cl_bndryonly = 0.8f;
        static constexpr float cl_domain = 0.5f;
        static constexpr float cl_inside = 0.15f;
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
        hgv->zScale.set_params (0,0); // makes the output flat in z direction, but you still get the colours
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
        std::cerr << "Caught exception reading svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << mplot::tools::getPwd() << std::endl;
        rtn = -1;
    }

    return rtn;
}
