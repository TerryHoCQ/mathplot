/*
 * Visualize a rod and boxes (for debugging of rotations about scene objects)
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <stdexcept>
#include <string>

#include <sm/vec>
#include <sm/config>

#include <mplot/Visual.h>
#include <mplot/RodVisual.h>
#include <mplot/ColourMap.h>

int main (int argc, char** argv)
{
    int rtn = -1;

    sm::config conf("/tmp/tube_box.json");
    conf.process_args (argc, argv);

    // Pass on cmd line
    sm::vec<float, 3> b1 = conf.getvec<float, 3> ("b1");
    sm::vec<float, 3> b2 = conf.getvec<float, 3> ("b2");
    sm::vec<float, 3> b3 = conf.getvec<float, 3> ("b3");
    sm::vec<float, 3> b4 = conf.getvec<float, 3> ("b4");
    sm::vec<float, 3> b5 = conf.getvec<float, 3> ("b5");
    sm::vec<float, 3> b6 = conf.getvec<float, 3> ("b6");
    sm::vec<float, 3> b7 = conf.getvec<float, 3> ("b7");
    sm::vec<float, 3> b8 = conf.getvec<float, 3> ("b8");

    mplot::Visual v(1024, 768, "Tube and box");

    v.lightingEffects (true);
    v.coordArrowsInScene (true);
    v.updateCoordLengths ({ 2.0f, 2.0f, 3.0f }, 0.8f);

    try {
        sm::vec<float, 3> offset = { 0.0, 0.0,  0.0 };
        sm::vec<float, 3> start =  { 0.1, 0.1,  10 };
        sm::vec<float, 3> end =    { 0.1, 0.1, -10 };

        // The 'rod' acting as our user line. Maroon is end, which is z = -10.
        auto rvm = std::make_unique<mplot::RodVisual<>> (offset, start, end, 0.05f, mplot::colour::black, mplot::colour::maroon3);
        v.bindmodel (rvm);
        rvm->face_uy = sm::vec<>::ux();
        rvm->face_uz = sm::vec<>::uy();
        rvm->finalize();
        v.addVisualModel (rvm);

        mplot::ColourMap<float> cm (mplot::ColourMapType::Jet);
        auto cl = cm.convert (0/3.0f);

        // The 'boxes'
        rvm = std::make_unique<mplot::RodVisual<>>(offset, b1, b2, 0.05f, cl);
        v.bindmodel (rvm);
        rvm->show_bb (true);
        rvm->colour_bb = cl;
        rvm->finalize();
        auto boxA = v.addVisualModel (rvm);

        cl = cm.convert (1/3.0f);
        rvm = std::make_unique<mplot::RodVisual<>>(offset, b3, b4, 0.05f, cl);
        v.bindmodel (rvm);
        rvm->show_bb (true);
        rvm->colour_bb = cl;
        rvm->finalize();
        auto boxB = v.addVisualModel (rvm);

        cl = cm.convert (2/3.0f);
        rvm = std::make_unique<mplot::RodVisual<>>(offset, b5, b6, 0.05f, cl);
        v.bindmodel (rvm);
        rvm->show_bb (true);
        rvm->colour_bb = cl;
        rvm->finalize();
        auto boxC = v.addVisualModel (rvm);

        cl = cm.convert (3/3.0f);
        rvm = std::make_unique<mplot::RodVisual<>>(offset, b7, b8, 0.05f, cl);
        v.bindmodel (rvm);
        rvm->show_bb (true);
        rvm->colour_bb = cl;
        rvm->finalize();
        auto boxD = v.addVisualModel (rvm);

        while (!v.readyToFinish()) {
            v.waitevents (0.03);
            try {
                sm::config conf("/tmp/tube_box.json");
                if (conf.ready) {
                    sm::vec<float, 3> _b1 = conf.getvec<float, 3> ("b1");
                    sm::vec<float, 3> _b2 = conf.getvec<float, 3> ("b2");
                    boxA->update (_b1, _b2);
                    sm::vec<float, 3> _b3 = conf.getvec<float, 3> ("b3");
                    sm::vec<float, 3> _b4 = conf.getvec<float, 3> ("b4");
                    boxB->update (_b3, _b4);
                    sm::vec<float, 3> _b5 = conf.getvec<float, 3> ("b5");
                    sm::vec<float, 3> _b6 = conf.getvec<float, 3> ("b6");
                    boxC->update (_b5, _b6);
                    sm::vec<float, 3> _b7 = conf.getvec<float, 3> ("b7");
                    sm::vec<float, 3> _b8 = conf.getvec<float, 3> ("b8");
                    boxD->update (_b7, _b8);
                }
            } catch (const std::exception& e) {}
            v.render();
        }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
