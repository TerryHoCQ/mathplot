/*
 * Visualize bounding boxes from user's view. Requires another mathplot program running with:
 *
 * v.options.set (mplot::visual_options::boundingBoxesToJson, true);
 *
 * The geodesic.cpp program provides an example.
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

    std::string jsonpath = "/tmp/mathplot_bounding_boxes.json";

    sm::config conf(jsonpath);
    conf.process_args (argc, argv);

    uint32_t n = conf.get<uint32_t>("n", 0);
    uint32_t n_models = n / 2;

    mplot::Visual v(1024, 768, "Bounding boxes from another mathplot");

    v.lightingEffects (true);
    v.coordArrowsInScene (true);
    v.updateCoordLengths ({ 2.0f, 2.0f, 3.0f }, 0.8f);

    constexpr sm::vec<float, 3> offset = { 0.0, 0.0,  0.0 };
    sm::vec<float, 3> start =  { 0.1, 0.1,  100 };
    sm::vec<float, 3> end =    { 0.1, 0.1, -100 };

    // The 'rod' acting as our user line. Maroon is end, which is z = -10.
    auto rvm = std::make_unique<mplot::RodVisual<>> (offset, start, end, 0.05f, mplot::colour::black, mplot::colour::maroon3);
    v.bindmodel (rvm);
    rvm->face_uy = sm::vec<>::ux();
    rvm->face_uz = sm::vec<>::uy();
    rvm->finalize();
    v.addVisualModel (rvm);

    mplot::ColourMap<float> cm (mplot::ColourMapType::Jet);

    std::vector<mplot::RodVisual<>*> pointers (n_models, nullptr);

    // The 'boxes'
    for (uint32_t i = 0; i < n_models; ++i) {

        // Pass on cmd line
        std::string tag1 = "b" + std::to_string(i * 2 + 1);
        std::string tag2 = "b" + std::to_string(i * 2 + 2);
        sm::vec<float, 3> b1 = conf.getvec<float, 3> (tag1);
        sm::vec<float, 3> b2 = conf.getvec<float, 3> (tag2);

        rvm = std::make_unique<mplot::RodVisual<>>(offset, b1, b2, 0.05f, cm.convert (i / static_cast<float>(n_models - 1)));
        v.bindmodel (rvm);
        rvm->show_bb (true);
        rvm->colour_bb = cm.convert (i / static_cast<float>(n_models - 1));
        rvm->finalize();
        pointers[i] = v.addVisualModel (rvm);
    }

    while (!v.readyToFinish()) {
        v.waitevents (0.03);
        try {
            sm::config conf(jsonpath);
            if (conf.ready) {
                for (uint32_t i = 0; i < n_models; ++i) {
                    std::string tag1 = "b" + std::to_string(i * 2 + 1);
                    std::string tag2 = "b" + std::to_string(i * 2 + 2);
                    sm::vec<float, 3> _b1 = conf.getvec<float, 3> (tag1);
                    sm::vec<float, 3> _b2 = conf.getvec<float, 3> (tag2);
                    pointers[i]->update (_b1, _b2);
                }
            }
        } catch (const std::exception& e) {}
        v.render();
    }

    return rtn;
}
