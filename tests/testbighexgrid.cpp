/*
 * Test a big hex grid with many hexes. Apply boundary as an ellipse.
 */

#include <utility>
#include <iostream>
#include <fstream>
#include <cmath>

#include <sm/vec>
#include <sm/hexgrid>

#include <mplot/Visual.h>
#include <mplot/VisualDataModel.h>
#include <mplot/HexGridVisual.h>
#include <mplot/ReadCurves.h>
#include <mplot/tools.h>

int main()
{
    int rtn = -1;

    mplot::Visual v(800,600,"Test window");
    v.zNear = 0.001;

    try {
        sm::hexgrid hg(0.002, 8, 0);
        hg.setEllipticalBoundary (1.6,2);

        std::cout << hg.extent() << std::endl;

        std::cout << "Number of hexes in grid:" << hg.num() << std::endl;
        std::cout << "Last vector index:" << hg.lastVectorIndex() << std::endl;

        if (hg.num() != 1604) { rtn = -1; }

        std::vector<float> data;
        unsigned int nhex = hg.num();
        data.resize(nhex, 0.0);

        // Make some dummy data (a sine wave)
        for (unsigned int hi=0; hi<nhex; ++hi) {
            data[hi] = 0.5 + 0.5*std::sin(10*hg.d_x[hi]); // Range 0->1
        }
        std::cout << "Created " << data.size() << " floats in data" << std::endl;

        sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };
        auto hgv = std::make_unique<mplot::HexGridVisual<float>> (&hg, offset);
        v.bindmodel (hgv);
        hgv->hexVisMode = mplot::HexVisMode::Triangles; // Triangles faster to render than the default hexes
        hgv->setScalarData (&data);
        hgv->zScale.set_params (0.1f, 0.0f);
        hgv->finalize();
        v.addVisualModel (hgv);

        v.render();

        while (v.readyToFinish() == false) { v.keepOpen(); }

    } catch (const std::exception& e) {
        std::cerr << "Caught exception reading trial.svg: " << e.what() << std::endl;
        std::cerr << "Current working directory: " << mplot::tools::getPwd() << std::endl;
        rtn = -1;
    }


    return rtn;
}
