/*
 * A compound ray eye viewer
 *
 * Demonstrating use of mplot::compoundray::EyeVisual
 */

#include <iostream>
#include <string>
#include <memory>

#include <sm/vec>
#include <sm/vvec>

#include <mplot/Visual.h>
#include <mplot/ColourMap.h>
#include <mplot/SphereVisual.h>
#include <mplot/VectorVisual.h>
#include <mplot/compoundray/EyeVisual.h>

int main (int argc, char** argv)
{
    std::string eyefile = "";
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " path/to/eyefile.eye [projection sphere radius]\n";
        return -1;
    } else {
        eyefile = std::string (argv[1]);
    }

    float psrad = 0.6f;
    if (argc > 2) { psrad = std::atof (argv[2]); }

    auto v = mplot::Visual<>(1024, 768, "mplot::compoundray::EyeVisual");

    // We read the information from the eye file into a vector of Ommatidium objects.  Ommatidium is
    // defined in "cameras/CompoundEyeDataTypes.h" in compound ray, mplot::Ommatidium is a
    // mplot/Seb's maths style equivalent. It contains 2 3D float vectors and two scalar floating point
    // values.
    auto ommatidia = std::make_unique<std::vector<mplot::compoundray::Ommatidium>>();
    std::vector<std::array<float, 3>> ommatidiaColours;

    // Read the eye file into ommatidia data structure. compound ray does this internally when we're
    // using it, but for this example we instead make use of mplot::compoundray::readEye
    if (mplot::compoundray::readEye (ommatidia.get(), eyefile) == nullptr) { std::cout << "Failed to read eye\n"; return -1; }

    // Make some dummy data to demo the eye
    sm::vvec<float> ommatidiaData;
    ommatidiaData.linspace (0, 1, ommatidia->size());
    // Colour it with a colour map
    mplot::ColourMap cm (mplot::ColourMapType::Plasma);
    ommatidiaColours.resize (ommatidia->size());
    for (size_t i = 0; i < ommatidia->size(); ++i) {
        ommatidiaColours[i] = cm.convert (ommatidiaData[i]);
    }

    // A second eye goes in the 'eye only' window
    //mplot::compoundray::EyeVisual<>* ep = nullptr;
    auto eyevm = std::make_unique<mplot::compoundray::EyeVisual<>> (sm::vec<>{}, &ommatidiaColours, ommatidia.get());
    v.bindmodel (eyevm);
    eyevm->name = "Big Eye";
    eyevm->show_cones = false;
    eyevm->proj_sphere_centre = { 0, 0, 0 };
    eyevm->proj_sphere_radius = psrad;
    eyevm->twod_offset = { 0, 2, 0 };
    eyevm->show_sphere = true;
    eyevm->show_rays = true;
    eyevm->finalize();

    v.addVisualModel (eyevm);

#if 0
    ep = v.addVisualModel (eyevm);
    if (ep->show_sphere) {
        auto svm = std::make_unique<mplot::SphereVisual<>> (ep->proj_sphere_centre, ep->proj_sphere_radius, mplot::colour::slategray1);
        v.bindmodel (svm);
        svm->setAlpha (0.5);
        svm->finalize();
        v.addVisualModel (svm);


        for (size_t i = 0; i < ommatidia->size(); ++i) {
            // Can now find intersections on our sphere
            sm::vec<> l0 = (*ommatidia)[i].relativePosition;
            sm::vec<> l = -(*ommatidia)[i].relativeDirection;

            // Show direction vector from ommatidium position
            auto vvm = std::make_unique<mplot::VectorVisual<float, 3>> (l0);
            v.bindmodel (vvm);
            vvm->vgoes = mplot::VectorGoes::FromOrigin;
            vvm->thickness = 0.001f;
            vvm->thevec = l;
            vvm->finalize();
            v.addVisualModel (vvm);

            sm::vec<sm::vec<>, 2> intersections = sm::geometry::ray_sphere_intersection (sm::vec<>{}, ep->proj_sphere_radius, l0, l);

            if (intersections[0][0] != std::numeric_limits<float>::max()) {
                // intersections[0] is the coordinate for the ommatidia pixel on the sphere
                auto ivm1 = std::make_unique<mplot::SphereVisual<>> (intersections[0], 0.01f * psrad, mplot::colour::crimson);
                v.bindmodel (ivm1);
                ivm1->finalize();
                v.addVisualModel (ivm1);
            }
#if 0
            if (intersections[1][0] != std::numeric_limits<float>::max()) {
                auto ivm2 = std::make_unique<mplot::SphereVisual<>> (intersections[1], 0.01f * psrad, mplot::colour::blue);
                v.bindmodel (ivm2);
                ivm2->finalize();
                v.addVisualModel (ivm2);
            }
#endif
        }
    }
#endif
    v.keepOpen();
}
