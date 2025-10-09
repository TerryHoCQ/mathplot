// Draw a cube with RhomboVisual and then make up vectors to transform with mat44s

#include <sm/vec>
#include <sm/mat44>
#include <mplot/compoundray/Visual.h>
#include <mplot/RhomboVisual.h>
#include <mplot/VectorVisual.h>
#include <mplot/SphereVisual.h>

#include <Eigen/Dense>
#include <Eigen/Geometry>

int main()
{
    // Create a scene
    mplot::compoundray::Visual v(1024, 768, "A cube");
    v.showCoordArrows (true);
    v.coordArrowsInScene (true);
    v.lightingEffects();

    // Parameters of the model
    sm::vec<float, 3> offset = { 0, 0, 0 };   // a within-scene offset

    sm::vec<float, 3> e1 = { 1, 0, 0 };
    sm::vec<float, 3> e2 = { 0, 1, 0 };
    sm::vec<float, 3> e3 = { 0, 0, 1 };

    sm::vec<float, 3> colour1 = { 0.35,  0.76,  0.98 };  // RGB colour triplet

    auto rv = std::make_unique<mplot::RhomboVisual<>> (offset, e1, e2, e3, colour1);
    v.bindmodel (rv);
    rv->name = "Cube.002";
    rv->facecm = mplot::ColourMapType::Rainbow; // Try Rainbow, Batlow, Tofino
    rv->annotate = true;
    rv->finalize();
    v.addVisualModel (rv);

    // Native locations/vectors
    sm::vec<> l1 = { 0.8, 1, 0.5 }; // start location
    sm::vec<> mv1 = { 0.2, 0, 0 };  // movement to edge
    sm::vec<> mv2 = { 0.1, 0, 0 };  // movement past edge
    sm::vec<> ra = { 0, 0, -1 };    // rotn axis
    sm::vec<> d1_s = { 0, 0, 0 };
    sm::vec<> d1_e = { 0.3, 0, 0 }; // Direction at l1 - our step length
    sm::vec<> d1 = d1_e - d1_s;

    auto rotang = sm::mathconst<float>::pi / 2;

    // Eigen copies
    Eigen::Vector3f el1 (l1[0], l1[1], l1[2]);
    Eigen::Vector3f emv1 (mv1[0], mv1[1], mv1[2]);
    Eigen::Vector3f emv2 (mv2[0], mv2[1], mv2[2]);
    Eigen::Vector3f era (ra[0], ra[1], ra[2]);
    Eigen::Vector3f ed1_s (d1_s[0], d1_s[1], d1_s[2]);
    Eigen::Vector3f ed1_e (d1_e[0], d1_e[1], d1_e[2]);

    // mat44 transformation
    sm::mat44<float> m1;
    m1.translate (mv1);
    m1.translate (-(l1 + mv1));
    m1.rotate (ra, rotang);
    //m1.translate (l1 + mv1);
    //m1.translate (mv2);

    sm::vec<> l2 = (m1 * l1).less_one_dim();
    sm::vec<> d2_s = (m1 * d1_s).less_one_dim();
    sm::vec<> d2_e = (m1 * d1_e).less_one_dim();
    sm::vec<> d2 = d2_e - d2_s;

    // Eigen transformation
    Eigen::Transform<float, 3, Eigen::TransformTraits::Affine> em1;
    em1.setIdentity();
    em1.translate (emv1);
    em1.translate (-(el1 + emv1));
    em1.rotate (Eigen::AngleAxisf(rotang, era));
    //em1.translate (el1 + emv1);
    //em1.translate (emv2);

    Eigen::Vector3f el2 = (em1 * el1);
    Eigen::Vector3f ed2_s = em1 * ed1_s;
    Eigen::Vector3f ed2_e = em1 * ed1_e;
    Eigen::Vector3f ed2 = ed2_e - ed2_s;
    sm::vec<> eig_l2 = { el2[0], el2[1], el2[2] };
    std::cout << "l2 = " << l2 << " whereas eig_l2 = " << eig_l2 << std::endl;
    sm::vec<> eig_d2 = { ed2[0], ed2[1], ed2[2] };


    auto sv = std::make_unique<mplot::SphereVisual<>>(l1, 0.005, mplot::colour::magenta);
    v.bindmodel (sv);
    sv->finalize();
    v.addVisualModel (sv);

    auto vvm = std::make_unique<mplot::VectorVisual<float, 3>>(l1);
    v.bindmodel (vvm);
    vvm->thevec = d1;
    vvm->vgoes = mplot::VectorGoes::FromOrigin;
    vvm->thickness *= 0.02;
    vvm->fixed_colour = true;
    vvm->single_colour = mplot::colour::crimson;
    vvm->finalize();
    v.addVisualModel (vvm);

    sv = std::make_unique<mplot::SphereVisual<>>(l2, 0.01, mplot::colour::goldenrod3);
    v.bindmodel (sv);
    sv->finalize();
    v.addVisualModel (sv);

    vvm = std::make_unique<mplot::VectorVisual<float, 3>>(l2);
    v.bindmodel (vvm);
    vvm->thevec = d2;
    vvm->vgoes = mplot::VectorGoes::FromOrigin;
    vvm->thickness *= 0.02;
    vvm->fixed_colour = true;
    vvm->single_colour = mplot::colour::blue;
    vvm->finalize();
    v.addVisualModel (vvm);

    sv = std::make_unique<mplot::SphereVisual<>>(eig_l2, 0.01, mplot::colour::mediumpurple1);
    v.bindmodel (sv);
    sv->finalize();
    v.addVisualModel (sv);

    vvm = std::make_unique<mplot::VectorVisual<float, 3>>(eig_l2);
    v.bindmodel (vvm);
    vvm->thevec = eig_d2;
    vvm->vgoes = mplot::VectorGoes::FromOrigin;
    vvm->thickness *= 0.02;
    vvm->fixed_colour = true;
    vvm->single_colour = mplot::colour::cadetblue1;
    vvm->finalize();
    v.addVisualModel (vvm);

    v.keepOpen();

    return 0;
}
