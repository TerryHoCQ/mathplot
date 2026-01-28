/*
 * Visualize a scatter of spheres with the duochrome colour map
 */
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

#include <sm/scale>
#include <sm/vec>

#include <mplot/Visual.h>
#include <mplot/ColourMap.h>
#include <mplot/ScatterVisual.h>

int main()
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "ScatterVisual with duochrome colourmap");
    v.zNear = 0.001f;

    static constexpr int slen = 20;
    static constexpr int half_slen = slen/2;
    try {
        sm::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        sm::scale<float> scale;
        scale.set_params (1.0, 0.0);

        std::vector<sm::vec<float, 3>> points(slen*slen);
        std::vector<sm::vec<float, 3>> vecdata(slen*slen);
        std::vector<float> data(slen*slen);
        size_t k = 0;
        for (int i = -half_slen; i < half_slen; ++i) {
            for (int j = -half_slen; j < half_slen; ++j) {
                float x = 0.1f * i;
                float y = 0.1f * j;
                // z is some function of x, y
                float z = 0;//x * std::exp(-(x*x) - (y*y));
                points[k] = {x, y, z};
                data[k] = z;
                k++;
            }
        }

        auto sv = std::make_unique<mplot::ScatterVisual<float>> (offset);
        v.bindmodel (sv);
        sv->setDataCoords (&points);
        sv->setScalarData (&data);
        // Set the vector data to the coordinates - we'll visualize duochrome based on x and y
        sv->setVectorData (&points);
        sv->radiusFixed = 0.035f;
        sv->colourScale = scale;
#if 1
        sv->cm.setType (mplot::ColourMapType::Duochrome);
        sv->cm.setHueGB();
#else
        // You can alternatively use a 1D colour map like Plasma with the map set to "act 2D"
        sv->cm.setType (mplot::ColourMapType::Plasma);
        sv->cm.set_act_2d (true);
#endif
        sv->finalize();
        v.addVisualModel (sv);

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
