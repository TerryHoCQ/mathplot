#include <utility>
#include <iostream>
#include <cmath>

#include <sm/hexgrid>
#include <sm/vec>

#include <mplot/Visual.h>
#include <mplot/HexGridVisual.h>
#include <mplot/ColourMap.h>
#include <mplot/tools.h>

int main (int argc, char** argv)
{
    int rtn = 0;

    mplot::Visual v(1024, 768, "Ellipse");
    v.zNear = 0.001;

    bool holdVis = false;
    if (argc > 1) {
        std::string a1(argv[1]);
        if (a1.size() > 0) { holdVis = true; }
    }
    std::cout << "NB: Provide a cmd line arg (anything) to see the graphical window for this program" << std::endl;

    try {
        sm::hexgrid hg(0.01, 3.0f, 0.0f);
        hg.setEllipticalBoundary (1, 0.7);

        std::cout << hg.extent() << std::endl;
        std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
        std::cout << "Last vector index:" << hg.lastVectorIndex() << std::endl;

        if (hg.num() != 25717) { rtn = -1; }

        std::vector<float> data;
        unsigned int nhex = hg.num();
        data.resize(nhex, 0.0);

        // Make some dummy data (a sine wave)
        for (unsigned int hi=0; hi<nhex; ++hi) {
            data[hi] = 0.5 + 0.5*sin(hg.d_x[hi]); // Range 0->1
        }
        std::cout << "Created " << data.size() << " floats in data" << std::endl;

        sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };
        auto hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, offset);
        v.bindmodel (hgv);
        hgv->setScalarData (&data);
        hgv->cm.setType (mplot::ColourMapType::Magma);
        hgv->zScale.set_params (0.0f, 0.0f);
        hgv->finalize();
        v.addVisualModel (hgv);
        v.render();

        if (holdVis == true) { v.keepOpen(); }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << mplot::tools::getPwd() << std::endl;
        rtn = -1;
    }

    return rtn;
}
