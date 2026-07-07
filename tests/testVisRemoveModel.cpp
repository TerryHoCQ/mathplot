/*
 * Visualize a quiver field
 */
#include <memory>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

import sm.vec;
import sm.scale;

import mplot.visual;
import mplot.quivervisual;
import mplot.scattervisual;

int main (int argc, char** argv)
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "Visualization");
    v.zNear = 0.001;
    v.showCoordArrows (true);
    // For a white background:
    v.backgroundWhite();

    bool holdVis = false;
    if (argc > 1) {
        std::string a1(argv[1]);
        if (a1.size() > 0) {
            holdVis = true;
        }
    }
    std::cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << std::endl;

    try {
        sm::vec<float, 3> offset = { 0.0, 0.0, 0.0 };

        std::vector<sm::vec<float, 3>> coords;
        coords.push_back ({0, 0,   0});
        coords.push_back ({1, 1,   0});
        coords.push_back ({2, 0,   0});
        coords.push_back ({1, 0.8, 0});
        coords.push_back ({2, 0.5, 0});

        std::vector<sm::vec<float, 3>> quivs;
        quivs.push_back ({0.3,   0.4,  0});
        quivs.push_back ({0.1,   0.2,  0.1});
        quivs.push_back ({-0.1,  0,    0});
        quivs.push_back ({-0.04, 0.05, -.2});
        quivs.push_back ({0.3,  -0.1,  0});

        auto qvp = std::make_unique<mplot::QuiverVisual<float>> (&coords, offset, &quivs, mplot::ColourMapType::Cividis);
        qvp->set_parent (v.get_id());
        qvp->finalize();
        unsigned int visId = v.addVisualModelId (qvp);
        std::cout << "Added Visual with visId " << visId << std::endl;

        offset = { 0.0, 0.1, 0.0 };
        sm::scale<float> scale;
        scale.set_params (1.0, 0.0);

        std::vector<sm::vec<float, 3>> points;
        points.push_back ({0,0,0});
        points.push_back ({1,1,0});
        points.push_back ({2,2.2,0});
        points.push_back ({3,2.8,0});
        points.push_back ({4,3.9,0});
        std::vector<float> data = {0.1, 0.2, 0.5, 0.6, 0.95};

        auto sv = std::make_unique<mplot::ScatterVisual<float>> (offset);
        sv->set_parent (v.get_id());
        sv->setDataCoords (&points);
        sv->setScalarData (&data);
        sv->radiusFixed = 0.03f;
        sv->colourScale = scale;
        sv->cm.setType (mplot::ColourMapType::Plasma);
        sv->finalize();
        mplot::ScatterVisual<float>* visPtr = v.addVisualModel (sv);

        v.render();
        // 10 seconds of viewing the quivers
        if (holdVis == true) {
            for (size_t ti = 0; ti < (size_t)std::round(10.0/0.018); ++ti) {
                v.waitevents (0.018);
                v.render();
            }
        }

        std::cout << "Remove model " << visId << " (the quivers)" << std::endl;
        v.removeVisualModel (visId);

        // 10 seconds of viewing the remaining scatter plot
        if (holdVis == true) {
            for (size_t ti = 0; ti < (size_t)std::round(10.0/0.018); ++ti) {
                v.waitevents (0.018);
                v.render();
            }
        }

        std::cout << "Remove scatter model with a pointer" << std::endl;
        v.removeVisualModel (visPtr);

        v.render();
        if (holdVis == true) { v.keepOpen(); }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
