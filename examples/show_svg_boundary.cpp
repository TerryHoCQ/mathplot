/*
 * Read the given svg file (first argument on command line), then
 * create a HexGrid and show the boundary.
 *
 * Useful to demonstrate that the mathplot code can read your
 * Adobe Illustrator or Inkscape generated SVG file.
 *
 * Author: Seb James <seb.james@sheffield.ac.uk>
 *
 * Date: June 2019
 */

#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <cstdlib>

#include <sm/bezcoord>
#include <sm/hexgrid>

#include <mplot/ReadCurves.h>
#include <mplot/tools.h>
#include <mplot/ColourMap.h>
#include <mplot/Visual.h>
#include <mplot/HexGridVisual.h>

int main(int argc, char** argv)
{
    int rtn = -1;

    if (argc < 2 && argc > 0) {
        std::cerr << "Usage: " << argv[0]
                  << " ./path/to/curves.svg [domain-span (mm default:3)] [hexdia (mm default:0.01)]" << std::endl;
        return rtn;
    }
    float gridspan = 3.0f;
    if (argc > 2) {
        gridspan = std::atof (argv[2]);
        std::cout << "User supplied grid width: " << gridspan << " mm" << std::endl;
    }
    float hexdia = 0.01f;
    if (argc > 3) {
        hexdia = std::atof (argv[3]);
        std::cout << "User supplied hex size: " << hexdia << " mm" << std::endl;
        if (hexdia < 0.003f) {
            std::cerr << "Very small hex dia - memory use may be large." << std::endl;
        }
    }

    try {
        // Read the curves
        mplot::ReadCurves r(argv[1]);
        // Create a HexGrid
        sm::hexgrid hg(hexdia, gridspan, 0);
        // Apply the curves as a boundary
        std::cout << "Number of hexes before setting boundary: " << hg.num() << std::endl;
        hg.setBoundary (r.getCorticalPath());

        std::cout << "hexgrid extent:" << std::endl;
        std::cout << "  x range: " << hg.getXmin() << " to " << hg.getXmax() << std::endl;
        std::cout << "  y range: " << hg.getXmin(90) << " to " << hg.getXmax(90) << std::endl;
        std::cout << "Scaling is " << r.getScale_mmpersvg() << " mm per SVG unit, or "
                  << r.getScale_svgpermm() << " units/mm" << std::endl;
        std::cout << "Number of hexes within the boundary: " << hg.num() << std::endl;

        // Display with mplot::Visual
        mplot::Visual v(1600, 1000, "Your SVG defined boundary");
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
        for (auto h : hg.hexen) {
            if (h.boundaryHex() && h.insideBoundary()) {
                // red is boundary hex AND inside boundary
                colours[h.vi] = cl_boundary_and_in;
            } else if (h.boundaryHex()) {
                // orange is boundary ONLY
                colours[h.vi] = cl_bndryonly;
            } else if (h.insideBoundary()) {
                // Inside boundary -  blue
                colours[h.vi] = cl_inside;
            } else {
                // The domain - greenish
                colours[h.vi] = cl_domain;
            }
        }
        hgv->cm.setType (mplot::ColourMapType::Jet);
        hgv->zScale.null_scaling(); // makes the output flat in z direction, but you still get the colours
        hgv->setScalarData (&colours);
        hgv->hexVisMode = mplot::HexVisMode::HexInterp; // Or mplot::HexVisMode::Triangles for a smoother surface plot
        hgv->finalize();
        v.addVisualModel (hgv);
        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading " << argv[1] << ": " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
