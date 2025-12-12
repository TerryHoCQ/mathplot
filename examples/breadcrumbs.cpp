/*
 * Visualize a test surface
 */
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>

#include <sm/scale>
#include <sm/vec>
#include <sm/vvec>

#include <mplot/Visual.h>
#include <mplot/ColourMap.h>
#include <mplot/InstancedScatterVisual.h>

// Instanced rendering requires OpenGL 4.3 or higher (for the SSBO)
constexpr int glver = mplot::gl::version_4_3;

int main()
{
    int rtn = -1;

    mplot::Visual<glver> v(1024, 768, "mplot::InstancedScatterVisual");
    v.lightingEffects();

    std::deque<sm::vec<float, 3>> points(50);
    std::deque<float> data(50);

    for (int i = 0; i < 50; ++i) {
        float x = i/50.0f;
        float y = std::sqrt (1.0f - x*x);
        // z is some function of x, y
        float z = x * std::exp(-(x*x) - (y*y));
        points[i] = {x, y, z};
        // std::cout << "points[" << i << "] = " << points[i] << std::endl;
        data[i] = z;
    }

    auto isv = std::make_unique<mplot::InstancedScatterVisual<glver>> (sm::vec<>{});
    v.bindmodel (isv);
    // Place data in SSBO
    isv->set_data (points, data);
    isv->radiusFixed = 0.03f;
    //isv->cm.setType (mplot::ColourMapType::Plasma);
    isv->finalize();
    v.addVisualModel (isv);

    while (!v.readyToFinish()) {
        v.render();
        v.wait (0.4);
        points.push_back (points.back() + sm::vec<>::uy() * 0.04f);
        points.pop_front();
        isv->set_data (points, data);
    }

    return rtn;
}
