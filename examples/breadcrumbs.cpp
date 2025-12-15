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

constexpr float st = sm::mathconst<float>::two_pi / 360;
constexpr float z = 0.0f;
sm::vec<float, 3> f (int i)
{
    float phi = st * i;
    float r = 1.0f + 0.1f * std::sin (phi * 10.0f);
    sm::vec<float, 3> xyz = {
        r * std::sin (phi),
        r * std::cos (phi),
        0.0f
    };
    return xyz;
}

int main()
{
    int rtn = -1;

    mplot::Visual<glver> v(1024, 768, "mplot::InstancedScatterVisual");
    v.lightingEffects();

    constexpr int dsz = 260;
    std::deque<sm::vec<float, 3>> points(dsz);
    std::deque<float> data(dsz);

    int i = 0;
    sm::vec<float, 3> xyz = {};
    for (i = 0; i < dsz; ++i) {
        xyz = f (i);
        points[i] = xyz;
        data[i] = xyz[2];
    }

    auto isv = std::make_unique<mplot::InstancedScatterVisual<glver>> (sm::vec<>{});
    v.bindmodel (isv);
    isv->radiusFixed = 0.03f;

    isv->finalize();
    auto isvp = v.addVisualModel (isv);

    while (!v.readyToFinish()) {
        v.render();
        v.wait (0.008);

        // Update all points/data
        xyz = f (i);
        points[i%dsz] = xyz;
        data[i%dsz] = xyz[2];
        ++i;
        // Place data in SSBO. first call of set_data should occur after first call to v.render()
        isvp->set_data (points, data);
    }

    return rtn;
}
