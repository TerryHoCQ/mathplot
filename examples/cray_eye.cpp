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

    float cx = 0.5f;
    if (argc > 2) { cx = std::atof (argv[2]); }

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

    auto eyevm = std::make_unique<mplot::compoundray::EyeVisual<>> (sm::vec<>{}, &ommatidiaColours, ommatidia.get());
    v.bindmodel (eyevm);
    eyevm->name = "Big Eye";
    eyevm->show_cones = false;

    sm::vec<> centre = { -0.5, 0, 0 };
    sm::vec<> twod_offset = { -.5, 2, 0 };
    eyevm->add_spherical_projection (twod_offset, mplot::compoundray::EyeVisual<>::projection_type::mercator, centre, psrad, 780, 1560);

    twod_offset = { .5, 2, 0 };
    centre = { cx, 0, 0 };
    eyevm->add_spherical_projection (twod_offset, mplot::compoundray::EyeVisual<>::projection_type::mercator, centre, psrad, 0, 780);

    eyevm->show_sphere = true;
    eyevm->show_rays = true;
    eyevm->finalize();

    [[maybe_unused]] auto ep = v.addVisualModel (eyevm);
    ep->reinitColours();

    v.keepOpen();
}
