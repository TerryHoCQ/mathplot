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

    sm::vvec<sm::vec<float, 3>> points(20*20);
    sm::vvec<float> alpha(20*20, 1.0f);
    sm::vvec<float> scale(20*20);
    sm::vvec<std::array<float, 3>> clrs(20*20);
    size_t k = 0;

    mplot::ColourMap<float> cm (mplot::ColourMapType::Plasma);
    for (int i = -10; i < 10; ++i) {
        for (int j = -10; j < 10; ++j) {
            float x = 0.1*i;
            float y = 0.1*j;
            // z is some function of x, y
            float z = x * std::exp(-(x*x) - (y*y));
            points[k] = {x, y, z};
            scale[k] = 1.0f + z;
            clrs[k] = cm.convert (z);
            k++;
        }
    }

    auto isv = std::make_unique<mplot::InstancedScatterVisual<glver>> (sm::vec<>{});
    v.bindmodel (isv);
    isv->radiusFixed = 0.03f;
    isv->finalize();
    auto isvp = v.addVisualModel (isv);

    v.render();
    // We set the instance data, which adds points, colours, alpha and scale
    isvp->set_instance_data (points, clrs, alpha, scale);

    v.keepOpen();

    return rtn;
}
