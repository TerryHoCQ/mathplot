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
#include <mplot/GeodesicVisual.h>

// Instanced rendering requires OpenGL 4.3 or higher (for the SSBO)
constexpr int glver = mplot::gl::version_4_3;

constexpr float st = sm::mathconst<float>::two_pi / 360;
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

    // A normal, non instanced model. A sphere to orbit around.
    auto gv1 = std::make_unique<mplot::GeodesicVisual<float,glver>> (sm::vec<>{}, 0.2f);
    gv1->name = "geodesic";
    v.bindmodel (gv1);
    gv1->iterations = 3;
    gv1->cm.setType (mplot::ColourMapType::Tofino);
    gv1->finalize();
    // re-colour after construction
    auto gv1p = v.addVisualModel (gv1);
    // sequential colouring:
    size_t sz1 = gv1p->data.size();
    gv1p->data.linspace (0.0f, 1.0f, sz1);
    gv1p->reinitColours();

    constexpr int dsz = 260;
    sm::vvec<sm::vec<float, 3>> points(dsz);
    sm::vvec<float> psz(dsz);

    int i = 0;
    sm::vec<float, 3> xyz = {};
    for (i = 0; i < dsz; ++i) {
        xyz = f (i);
        points[i] = xyz;
        psz[i] = xyz[2];
    }

    auto isv = std::make_unique<mplot::InstancedScatterVisual<glver>> (sm::vec<>{});
    isv->name = "isv1";
    v.bindmodel (isv);
    isv->max_instances = dsz;
    isv->radiusFixed = 0.03f;
    isv->finalize();
    auto isvp = v.addVisualModel (isv);

    // Another one
    isv = std::make_unique<mplot::InstancedScatterVisual<glver>> (sm::vec<>{0,0.1,0});
    isv->name = "isv2";
    v.bindmodel (isv);
    isv->max_instances = dsz;
    isv->radiusFixed = 0.03f;
    isv->finalize();
    auto isvp2 = v.addVisualModel (isv);

    v.render();
    isvp->set_instance_data (points); // colour, alpha, scale

    isvp2->set_instance_data (points * 1.2f, mplot::colour::black, 0.7f, 1.0f);

    sm::vvec<std::array<float, 3>> col = { mplot::colour::blue, mplot::colour::springgreen };
    sm::vvec<float> alph = { 0.5f, 1.0f };
    // A scaling vector to make sequential instances have a different size
    sm::vvec<float> scl = { 1.0f, 1.2f };

    while (!v.readyToFinish()) {

        xyz = f (i%360);
        //std::cout << "i = " << i << ", i%dsz = " << (i%dsz) << ", i%360 = " << (i%360) << " f(i%360) = " << xyz << " -> points[" << (i%dsz)<< "]" <<  std::endl;

        points[i%dsz] = xyz;
        psz[i%dsz] = xyz[2];

#if 1
        // Update all points/psz
        // Place data in SSBO. first call of set_data must occur after first call to v.render()
        isvp->set_instance_data (points, col, alph, scl);

        // As well as scl, we have an applied-to-all instances scale (iscl) that is passed to the
        // s_matrix in the instance shader
        float iscl = 1.5f + 0.5f * std::sin (sm::mathconst<float>::two_pi * (static_cast<float>(i % 360) / 360.0f));
        isvp->set_instance_scale (iscl);

        // Can set scale of the black spheres based on distance to rotation centre. As they get
        // further away, they get larger so you can still see them.
        float iscl2 = v.get_d_to_rotation_centre() * 0.1f;
        isvp2->set_instance_scale (iscl2);

        v.render();
        v.waitevents (0.018);

#else // I haven't mastered updating a subrange of data in an SSBO
        // update circularly, change isvp->instance_start each time
        std::cout << "updating data with points[i%dsz] = " << points[i%dsz] << std::endl;
        isvp->update_instance_data (points, data, (i % dsz), (i % dsz));

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
