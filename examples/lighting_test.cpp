// This draws several objects with the different primitives. It then spins the diffuse light around
// so that you can insepect that all the objects are lit/shaded correctly.
#include <memory>
#include <array>
#include <cmath>

import sm.vec;
import mplot.visual;
import mplot.visualmodel;

template <int glver = mplot::gl::version_4_1>
struct my_vm : public mplot::VisualModel<glver>
{
    // Make a constructor that sets the location in the scene via a parent method
    my_vm(const sm::vec<float> _offset) : mplot::VisualModel<glver>::VisualModel (_offset) {}

    void initializeVertices()
    {
        this->computeRectCuboid ({-1.5,-0.5,-0.5}, 3, 1, 1, mplot::colour::navy);

        std::array<sm::vec<float>, 8> cubecorners;
        cubecorners[0] = {-0.1, 0.5, -0.1};
        cubecorners[1] = {-0.1, 0.5, 0.1};
        cubecorners[2] = {0.1, 0.5, 0.1};
        cubecorners[3] = {0.1, 0.5, -0.1};
        cubecorners[4] = {-0.2, 2, -0.2};
        cubecorners[5] = {-0.2, 2, 0.2};
        cubecorners[6] = {0.2, 2, 0.2};
        cubecorners[7] = {0.2, 2, -0.2};
        this->computeCuboid (cubecorners, mplot::colour::crimson);

        sm::vec<float> a1s = { 0.75, 0.5, 0 };
        sm::vec<float> a1e = { 1, 1.5, 0 };
        this->computeTube (a1s, a1e, mplot::colour::navy, mplot::colour::blue, 0.1f);

        this->computeSphereGeo (a1e, mplot::colour::orchid1, 0.3, 4);

        this->computeRing ({ 1, 2.5, 0 }, mplot::colour::mint, 0.3, 0.08, 50);

        this->computeFlaredTube ({ 0, 2.5, 0 }, { -2, 2.5, 1 },
                                 mplot::colour::blue2, mplot::colour::green2,
                                 0.23f, 8, sm::mathconst<float>::pi/20.0f);
    }
};

int main()
{
    mplot::Visual v(1024, 768, "Lighting demonstration");
    v.lightingEffects();

    // Make our model
    auto mvm = std::make_unique<my_vm<>>(sm::vec<float>{0,0,0});
    mvm->set_parent (v.get_id());
    mvm->finalize();
    v.addVisualModel (mvm);

    // Spin our light source around in a circle in fine increments
    constexpr float angle_inc = sm::mathconst<float>::two_pi / 600.0f;
    constexpr float light_r = 10.0f;
    float theta = 0.0f;
    float light_angle = 0.0f;
    while (!v.readyToFinish()) {
        v.waitevents (0.018);
        light_angle += angle_inc;
        if (light_angle >= sm::mathconst<float>::two_pi) {
            light_angle -= sm::mathconst<float>::two_pi;
            theta += sm::mathconst<float>::two_pi / 12.0f;
        }
        float x = light_r * std::cos (light_angle);
        float z = light_r * std::sin (light_angle);
        v.diffuse_position = { x, 5.0f * std::cos (theta), z };
        v.render();
    }

    return 0;
}
