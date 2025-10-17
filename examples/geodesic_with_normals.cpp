/*
 * Visualize a sequence of icosahedral geodesics, showing their normals
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <stdexcept>
#include <string>

#include <sm/vec>

#include <mplot/Visual.h>
#include <mplot/ColourMap.h>
#include <mplot/GeodesicVisual.h>
#include <mplot/NormalsVisual.h>

int main()
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "Geodesic Polyhedra with normals");
    v.showCoordArrows (true);
    // Set the Visual to rotate about the nearest VisualModel (Change at runtime with Ctrl-k)
    v.rotateAboutNearest (true);
    // In this example, use the 'rotate about a scene vertical axis' mode
    v.rotateAboutVertical (true);

    try {
        sm::vec<float, 3> offset = {};
        sm::vec<float, 3> step = { 2.2f };

        mplot::ColourMap<float> cm (mplot::ColourMapType::Jet);
        int imax = 4;
        for (int i = 0; i < imax; ++i) {
            auto cl = cm.convert (i / static_cast<float>(imax - 1));
            auto gv1 = std::make_unique<mplot::GeodesicVisual<float>> (offset + step * i, 0.9f);
            v.bindmodel (gv1);
            gv1->iterations = i;
            std::string lbl = std::string("iterations = ") + std::to_string(i);
            gv1->addLabel (lbl, {0, -1, 0}, mplot::TextFeatures(0.06f));
            gv1->cm.setType (mplot::ColourMapType::Jet);
            gv1->colour_bb = cl;
            gv1->finalize();

            // re-colour after construction
            auto gv1p = v.addVisualModel (gv1);
            float imax_mult = 1.0f / static_cast<float>(imax);
            // sequential colouring:
            size_t sz1 = gv1p->data.size();
            gv1p->data.linspace (0.0f, 1+i * imax_mult, sz1);
            gv1p->reinitColours();

            gv1p->vertex_postprocess(); // creates the triangles and normals required for NormalsVisual

            // Create an associate normals model
            auto nrm = std::make_unique<mplot::NormalsVisual<>> (gv1p);
            v.bindmodel (nrm);
            nrm->finalize();
            v.addVisualModel (nrm);
        }

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
