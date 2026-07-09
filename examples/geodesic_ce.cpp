/*
 * Visualize an Icosahedron using the Geodesic Visual that co-ops the unordered
 * constexpr geodesic function.
 */
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <stdexcept>
#include <string>
#include <memory>

import sm.vec;
import mplot.visual;
import mplot.geodesicvisualce;

int main()
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "(constexpr) Geodesic Polyhedra");
    v.showCoordArrows (true);
    v.lightingEffects (true);

    try {
        sm::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        sm::vec<float, 3> step = { 2.2, 0.0, 0.0 };

        auto gv1 = std::make_unique<mplot::GeodesicVisualCE<float, 0>> (offset + step * 0, 0.9f);
        gv1->set_parent (v.get_id());
        std::string lbl = std::string("iterations = 0");
        gv1->addLabel (lbl, {0, -1, 0}, mplot::TextFeatures(0.06f));
        gv1->finalize();
        v.addVisualModel (gv1);

        auto gv2 = std::make_unique<mplot::GeodesicVisualCE<float, 1>> (offset + step * 1, 0.9f);
        gv2->set_parent (v.get_id());
        lbl = std::string("iterations = 1");
        gv2->addLabel (lbl, {0, -1, 0}, mplot::TextFeatures(0.06f));
        gv2->finalize();
        v.addVisualModel (gv2);

        auto gv3 = std::make_unique<mplot::GeodesicVisualCE<float, 2>> (offset + step * 2, 0.9f);
        gv3->set_parent (v.get_id());
        lbl = std::string("iterations = 2");
        gv3->addLabel (lbl, {0, -1, 0}, mplot::TextFeatures(0.06f));
        gv3->finalize();
        v.addVisualModel (gv3);

        auto gv4 = std::make_unique<mplot::GeodesicVisualCE<float, 3>> (offset + step * 3, 0.9f);
        gv4->set_parent (v.get_id());
        lbl = std::string("iterations = 4");
        gv4->addLabel (lbl, {0, -1, 0}, mplot::TextFeatures(0.06f));
        gv4->finalize();
        v.addVisualModel (gv4);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
