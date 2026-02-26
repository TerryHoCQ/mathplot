/*
 * Demonstrating coordinate frame transformations
 */
#include <iostream>
#include <sm/vec>
#include <sm/mat>
#include <mplot/Visual.h>
#include <mplot/CoordArrows.h> // This is the only VisualModel derived class not called SomethingVisual!

int main()
{
    mplot::Visual v(1024, 768, "Slerping");
    v.lightingEffects (true);

    // A is the end location coordinate arrows (darker)
    sm::vec<float> offset = {};
    auto cavm = std::make_unique<mplot::CoordArrows<>> (offset);
    v.bindmodel (cavm);
    cavm->x_axis_col = mplot::colour::maroon;
    cavm->y_axis_col = mplot::colour::darkgreen;
    cavm->z_axis_col = mplot::colour::midnightblue;
    cavm->finalize();
    auto cA = v.addVisualModel (cavm);

    // B is the starting location coordinate arrows (lighter)
    cavm = std::make_unique<mplot::CoordArrows<>> (offset);
    v.bindmodel (cavm);
    cavm->x_axis_col = mplot::colour::firebrick1;
    cavm->y_axis_col = mplot::colour::palegreen3;
    cavm->z_axis_col = mplot::colour::steelblue1;
    cavm->finalize();
    auto cB = v.addVisualModel (cavm);

    cavm = std::make_unique<mplot::CoordArrows<>> (offset);
    v.bindmodel (cavm);
    cavm->finalize();
    auto cC = v.addVisualModel (cavm);

    // Define A's location as a viewmatrix. Apply a rotation (first) then a translation
    sm::mat<float, 4> A;
    A.translate (sm::vec<float>{0.2,0.4,0});
    A.rotate (sm::vec<float>::uy(), sm::mathconst<float>::pi_over_4);

    // The starting location.
    sm::mat<float, 4> B;
    B.translate (sm::vec<float>{1,1,1});
    B.rotate (sm::vec<float>::uz(), sm::mathconst<float>::pi_over_2);

    // From A and B find the pure translation between the two
    sm::vec<float> delta_x = A.translation() - B.translation();

    // Compute the pre-translation rotations
    sm::mat<float, 4> A0 = A;
    A0.translate (-A.translation());
    sm::mat<float, 4> B0 = B;
    B0.translate (-B.translation());
    sm::quaternion<float> rA = A0.rotation();
    sm::quaternion<float> rB = B0.rotation();

    // Set the viewmatrix for both A and B coordinate arrows (these will stay fixed)
    cA->setViewMatrix (A);
    cB->setViewMatrix (B);

    int count = 0;
    while (!v.readyToFinish()) {

        v.render();
        v.wait (0.018);

        float prop = static_cast<float>(count++ % 100) / 100.0f;

        // Compute the intermediate translation and rotation, C and apply this to the cC coordinate arrows
        sm::mat<float, 4> C;
        // C is a rotation of the intermediate rotation between rA and rB, then a translation to B +
        // the intermediate delta_x:
        C.translate (B.translation() + prop * delta_x);
        C.rotate (rB.slerp (rA, prop));

        cC->setViewMatrix (C);
    }
}
