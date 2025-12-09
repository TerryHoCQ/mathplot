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

    // Compound eyes are tiny. We make them larger in our scene by this factor
    constexpr float eye_scaleup = 1000.0f;

    float psrad = 0.5f;
    if (argc > 2) { psrad = std::atof (argv[2]); }

    // We read the information from the eye file into a vector of Ommatidium objects.  Ommatidium is
    // defined in "cameras/CompoundEyeDataTypes.h" in compound ray, mplot::Ommatidium is a
    // mplot/Seb's maths style equivalent. It contains 2 3D float vectors and two scalar floating point
    // values.
    auto ommatidia = std::make_unique<std::vector<mplot::compoundray::Ommatidium>>();
    std::vector<std::array<float, 3>> ommatidiaColours;

    // Read the eye file into ommatidia data structure. compound ray does this internally when we're
    // using it, but for this example we instead make use of mplot::compoundray::readEye
    if (mplot::compoundray::readEye (ommatidia.get(), eyefile) == nullptr) { std::cout << "Failed to read eye\n"; return -1; }

    sm::range<sm::vec<float>> ommspan = sm::range<sm::vec<float>>::search_initialized();
    // Use the eye spacing to control the size of the coord arrows
    for (auto omm : *ommatidia.get()) {
        // omm is an Ommatidium. Want to know the x, y and z spans
        ommspan.update (omm.relativePosition);
    }
    float ca_len = eye_scaleup * ommspan.span().max() / 3.0f;
    auto v = mplot::Visual<>(1024, 768, "mplot::compoundray::EyeVisual");
    v.showCoordArrows (true);
    v.coordArrowsInScene (true);
    v.updateCoordLengths ({ ca_len, ca_len, ca_len }, 1.0f);

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
    eyevm->name = "CompoundRay Eye";
    eyevm->show_cones = true;

    auto ptype = mplot::compoundray::EyeVisual<>::projection_type::equirectangular; // mercator, equirectangular or cassini
    sm::vec<> centre = { 0, 0, 0 };
    sm::mat44<float> twod_tr;
    twod_tr.translate (sm::vec<>{0,0,-0.1});
    eyevm->add_spherical_projection (ptype, twod_tr, centre, psrad);
    eyevm->pre_set_cone_length (4e-6f);
    eyevm->show_sphere = true;
    eyevm->show_rays = false;
    eyevm->finalize();

    [[maybe_unused]] auto ep = v.addVisualModel (eyevm);
    // ep->reinitColours();
    ep->scaleViewMatrix (eye_scaleup);

    v.keepOpen();
}
