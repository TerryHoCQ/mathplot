/*
 * Visualize a graph
 */
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

#include <sm/vec>
#include <sm/vvec>

#include <mplot/Visual.h>
#include <mplot/ColourMap.h>
#include <mplot/GraphVisual.h>

int main()
{
    namespace uc = mplot::unicode;

    int rtn = -1;

    mplot::Visual v(1024, 768, "Graph");
    v.zNear = 0.001;
    v.showCoordArrows (true);
    v.backgroundWhite();
    v.lightingEffects();

    try {
        sm::vvec<float> absc =  {-.5, -.4, -.3, -.2, -.1, 0, .1, .2, .3, .4, .5, .6, .7, .8};

        float step = 1.4f;
        float row2 = 1.2f;

        mplot::DatasetStyle ds;

        auto gv = std::make_unique<mplot::GraphVisual<float>>(sm::vec<float>({0,0,0}));
        v.bindmodel (gv);
        sm::vvec<float> data = absc.pow(3);
        sm::vec<float, 14> ardata;
        ardata.set_from (static_cast<std::vector<float>>(data));

        ds.linecolour =  {1.0, 0.0, 0.0};
        ds.linewidth = 0.015f;
        ds.markerstyle = mplot::markerstyle::triangle;
        ds.markercolour = {0.0, 0.0, 1.0};
        gv->setdata (absc, ardata, ds);

        gv->axisstyle = mplot::axisstyle::L;

        // Set xlabel to include the greek character alpha:
        gv->xlabel = "Include unicode symbols like this: " + uc::toUtf8 (uc::alpha);
        // A gamma - using raw unicode here instead of uc::gamma
        gv->ylabel = "Unicode for Greek gamma is 0x03b3: " + uc::toUtf8 (0x03b3);

        gv->setthickness (0.001f);
        gv->finalize();
        v.addVisualModel (gv);

        gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>({step,0,0}));
        v.bindmodel (gv);
        sm::vvec<float> data2 = absc.pow(2);
        ds.linecolour = {0.0, 0.0, 1.0};
        ds.markerstyle = mplot::markerstyle::hexagon;
        ds.markercolour = {0.0, 0.0, 0.0};
        gv->setdata (absc, data2, ds);
        gv->axisstyle = mplot::axisstyle::box;
        gv->ylabel = "mm";
        gv->xlabel = "Abscissa (notice that mm is not rotated)";
        gv->setthickness (0.005f);
        gv->finalize();
        v.addVisualModel (gv);

        gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>({0,-row2,0}));
        v.bindmodel (gv);
        sm::vvec<float> data3 = absc.pow(4);
        gv->setsize (1,0.8);
        ds.linecolour = {0.0, 1.0, 0.0};
        ds.markerstyle = mplot::markerstyle::circle;
        ds.markercolour = {0.0, 0.0, 1.0};
        ds.markersize = 0.02f;
        ds.markergap = 0.0f;
        gv->setdata (absc, data3, ds);
        gv->axisstyle = mplot::axisstyle::boxfullticks;
        gv->tickstyle = mplot::tickstyle::ticksin;
        gv->ylabel = "mmi";
        gv->xlabel = "mmi is just long enough to be rotated";
        gv->setthickness (0.001f);
        gv->finalize();
        v.addVisualModel (gv);

        gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>({step,-row2,0}));
        v.bindmodel (gv);
        absc.resize(1000, 0.0f);
        for (int i = 0; i < 1000; ++i) {
            absc[i] = static_cast<float>(i-500) * 0.01f;
        }
        sm::vvec<float> data4 = absc.pow(5);
        gv->setsize (1,0.8);
        ds.linecolour = {0.0, 0.0, 1.0};
        ds.markerstyle = mplot::markerstyle::none;
        ds.markergap = 0.0f;
        gv->setdata (absc, data4, ds);
        gv->axisstyle = mplot::axisstyle::cross;
        gv->setthickness (0.002f);
        gv->finalize();
        gv->twodimensional = false;
        v.addVisualModel (gv);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
