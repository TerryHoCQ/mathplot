/*
 * Visualize a single vector
 */
#include <iostream>
#include <array>
#include <stdexcept>
#include <string>

#include <sm/vec>
#include <sm/quaternion>
#include <sm/mat>

#include <mplot/Visual.h>
#include <mplot/ColourMap.h>
#include <mplot/VectorVisual.h>

int main()
{
    mplot::Visual v(1024, 768, "mplot::VectorVisual");
    v.lightingEffects();
    v.showCoordArrows (true);
    v.coordArrowsInScene (true);

    sm::vec<float> offset = {1,0,0};

    auto vvm = std::make_unique<mplot::VectorVisual<float, 3>>(offset);
    v.bindmodel (vvm);
    vvm->thevec = {1,1,1};
    vvm->fixed_colour = true;
    vvm->single_colour = mplot::colour::crimson;
    vvm->addLabel ("Rotn by quaternion", {-0.8, -0.5, 0}, mplot::TextFeatures(0.1f));
    vvm->finalize();
    auto ptr = v.addVisualModel (vvm);

    vvm = std::make_unique<mplot::VectorVisual<float, 3>>(-offset);
    v.bindmodel (vvm);
    vvm->thevec = {1,1,1};
    vvm->fixed_colour = true;
    vvm->single_colour = mplot::colour::royalblue;
    vvm->addLabel ("Rotn by 4x4 mat", {-0.8, -0.5, 0}, mplot::TextFeatures(0.1f));
    vvm->finalize();
    auto ptr2 = v.addVisualModel (vvm);

    float angle_per_frame = 0.05f;
    sm::vec<float> axis = {0,1,0.4};

    // quaternion way
    // Also demo quaternion rotation.
    // Set up a rotation about the z axis
    sm::quaternion<float> qr (axis, angle_per_frame);

    sm::mat<float, 4, 4> tf;
    tf.rotate (axis, angle_per_frame);

    while (!v.readyToFinish()) {
        v.render();
        v.wait (0.01);

        ptr->thevec = (qr * ptr->thevec);
        ptr2->thevec = (tf * ptr2->thevec).less_one_dim();

        ptr->reinit();
        ptr2->reinit();
    }

    return 0;
}
