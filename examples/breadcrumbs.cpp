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
sm::vec<float, 3> f (int i, float _z = 0.0f)
{
    float phi = st * i;
    float r = 1.0f + 0.1f * std::sin (phi * 10.0f);
    sm::vec<float, 3> xyz = {
        r * std::sin (phi),
        r * std::cos (phi),
        _z
    };
    return xyz;
}

int main()
{
    int rtn = -1;

    mplot::Visual<glver> v(1024, 768, "mplot::InstancedScatterVisual");
    v.lightingEffects();

    constexpr int dsz = 260;
    sm::vvec<sm::vec<float, 3>> points(dsz);
    sm::vvec<float> data(dsz);

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

    v.render();
    isvp->set_data (points, data);
    std::cout << "isvp->instance_count = " <<  isvp->instance_count << std::endl;

    while (!v.readyToFinish()) {

        xyz = f (i%360);
        //std::cout << "i = " << i << ", i%dsz = " << (i%dsz) << ", i%360 = " << (i%360) << " f(i%360) = " << xyz << " -> points[" << (i%dsz)<< "]" <<  std::endl;

        points[i%dsz] = xyz;
        data[i%dsz] = xyz[2];

#if 1
        // Update all points/data
        // Place data in SSBO. first call of set_data must occur after first call to v.render()
        isvp->set_data (points, data);

        v.render();
        v.waitevents (0.001);
#else
        // update circularly, change isvp->instance_start each time
        std::cout << "updating data with points[i%dsz] = " << points[i%dsz] << std::endl;
        isvp->update_data (points, data, (i % dsz), (i % dsz));

        std::cout << "Before render, isvp->instance_start is " << isvp->instance_start << std::endl;

        v.render();
        v.wait (0.4);

        isvp->instance_start += 1;
        if (isvp->instance_start >= isvp->instance_count) { isvp->instance_start = 0; }

#endif
        ++i;
    }

    return rtn;
}
