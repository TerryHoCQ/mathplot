/*
 * An example showing how to create two mplot::Visuals and then a third one.
 */
#include <memory>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cmath>

import mplot.visual;
import mplot.quivervisual;
import mplot.graphvisual;

int main()
{
    int rtn = -1;

    // Demonstrates use of offset (left at 0,0,0), lengths (3,2,1) and the 'thickness'
    // scaling factor (0.5) for the coordinate arrows. Defines, and makes current a new
    // window and OpenGL context.
    mplot::Visual v(1024, 768, "Close Window 1 and Window 3 briefly appears then program exits");
    v.showCoordArrows (true);
    v.showTitle (true);
    v.backgroundWhite();
    v.lightingEffects();

    { // I create two mplot::Visuals here in their own scope, so that I can demonstrate the creation
      // of a new, follow-on mplot::Visual at the end

        // v2 is a second window and OpenGL context
        mplot::Visual v2(768, 768, "Close Window 2 and Window 3 appears");
        v2.showCoordArrows (true);
        v2.showTitle (true);
        v2.backgroundWhite();
        v2.lightingEffects();

        try {
            // Set up data for the first Visual
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

            auto qvp = std::make_unique<mplot::QuiverVisual<float>>(&coords, offset, &quivs, mplot::ColourMapType::Cividis);
            qvp->set_parent (v.get_id());
            qvp->finalize();
            v.addVisualModel (qvp);

            // Set up v2 with a graph
            auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>({0,0,0}));
            gv->set_parent (v2.get_id());
            sm::vvec<float> x =  {-.5, -.4, -.3, -.2, -.1, 0, .1, .2, .3, .4, .5, .6, .7, .8};
            sm::vvec<float> y = x.pow(3);
            gv->setdata (x, y);
            gv->finalize();
            v2.addVisualModel (gv);

            while (v.readyToFinish() == false && v2.readyToFinish() == false) {
                v.waitevents (0.018);
                v2.render();
                v.render();
            }
            v.render();

        } catch (const std::exception& e) {
            std::cerr << "Caught exception: " << e.what() << std::endl;
            rtn = -1;
        }
    }

    v.render();

    // Both old windows have now gone out of scope. Right at the end, I re-create a mplot::Visual to
    // prove that it can be done (until March 11 2024, this would fail).
    mplot::Visual v3(1024, 768, "This is the third (empty) window");
    v3.showCoordArrows (true);
    v3.showTitle (true);
    v3.backgroundWhite();
    v3.lightingEffects();

    while (v3.readyToFinish() == false && v.readyToFinish() == false) {
        v3.waitevents (0.018);
        v.render();
        v3.render();
    }

    return rtn;
}
