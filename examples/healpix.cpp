/*
 * Make a healpix visual, showing the NEST index in a colour map
 */
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <sm/vec>
#include <mplot/Visual.h>
#include <mplot/HealpixVisual.h>
#include <mplot/SphericalProjectionVisual.h>

int main (int argc, char** argv)
{
    int ord = 7; // HEALPix order
    if (argc > 1) { ord = std::atoi (argv[1]); }

    mplot::Visual v(1024, 768, "Healpix");

    auto hpv = std::make_unique<mplot::HealpixVisual<float>> (sm::vec<float>{0,0,0});
    v.bindmodel (hpv);
    hpv->indicate_axes = true;
    hpv->set_order (ord);
    hpv->cm.setType (mplot::ColourMapType::Plasma);

    // The HealpixVisual has pixeldata, which is ordered with the NEST indexing
    // scheme. If we fill it with sequential values, then the colour map will show the
    // hierarchical nature of the HEALPix.
    for (int64_t p = 0; p < hpv->n_pixels(); ++p) {
        hpv->pixeldata[p] = static_cast<float>(p) / hpv->n_pixels();
    }

    std::stringstream ss;
    constexpr bool centre_horz = true;
    ss << ord << (ord == 1 ? "st" : (ord == 2 ? "nd" : (ord == 3 ? "rd" : "th")))
       << " order HEALPix with nside = " << hpv->get_nside()
       << " and " << hpv->n_pixels() << " pixels\n";
    hpv->addLabel (ss.str(), {0.0f, -1.2f , 0.0f }, mplot::TextFeatures{0.08f, centre_horz});

    // Finalize and add
    hpv->finalize();
    auto hpvp = v.addVisualModel (hpv);

    // Show some 2D projections, as well
    sm::vvec<std::array<float, 3>> hpvcolours (hpvp->pixeldata.size(), mplot::colour::crimson);
    sm::vvec<sm::vec<float, 2>> latlong (hpvp->pixeldata.size());
    for (uint32_t i = 0; i < hpvp->pixeldata.size(); ++i) {

        hp::t_ang ang = hp::nest2ang (hpvp->get_nside(), i);

        // ang.theta is co-latitude (and needs re-casting to be longitude), ang.phi is a longitude
        float lat = ang.theta - sm::mathconst<float>::pi_over_2;
        // constrain to range -pi/2 < lat <= pi/2
        lat *= 2.0f;
        sm::algo::minus_pi_to_pi (lat);
        lat /= 2.0f;

        latlong[i] = { lat, static_cast<float>(ang.phi) };

        hpvcolours[i] = hpvp->cm.convert (hpvp->pixeldata[i]);
    }

    // Add two-dimensional projections
    auto spv = std::make_unique<mplot::SphericalProjectionVisual<float>> (sm::vec<float>{4,0,0});
    v.bindmodel (spv);
    spv->twodimensional (true);
    spv->proj_type = sm::geometry::spherical_projection::type::mercator;
    spv->latlong = latlong;
    spv->colour = hpvcolours;
    spv->finalize();
    auto spvp = v.addVisualModel (spv);
    auto ext = spvp->extents(); // Use VisualModel::extents() to help place the label
    spvp->addLabel ("Mercator projection", sm::vec<>{ ext[0].min, ext[1].min - 0.16f, 0.0f }, mplot::TextFeatures(0.08f));

    spv = std::make_unique<mplot::SphericalProjectionVisual<float>> (sm::vec<float>{-5,-4,0});
    v.bindmodel (spv);
    spv->twodimensional (true);
    spv->proj_type = sm::geometry::spherical_projection::type::equirectangular;
    spv->latlong = latlong;
    spv->colour = hpvcolours;
    spv->finalize();
    spvp = v.addVisualModel (spv);
    ext = spvp->extents();
    spvp->addLabel ("Equirectangular projection", sm::vec<>{ ext[0].min, ext[1].min - 0.16f, 0.0f }, mplot::TextFeatures(0.08f));

    spv = std::make_unique<mplot::SphericalProjectionVisual<float>> (sm::vec<float>{-4,3,0});
    v.bindmodel (spv);
    spv->twodimensional (true);
    spv->proj_type = sm::geometry::spherical_projection::type::cassini;
    spv->latlong = latlong;
    spv->colour = hpvcolours;
    spv->finalize();
    spvp = v.addVisualModel (spv);
    ext = spvp->extents();
    spvp->addLabel ("Cassini projection", sm::vec<>{ ext[0].min, ext[1].min - 0.16f, 0.0f }, mplot::TextFeatures(0.08f));

    v.keepOpen();
    return 0;
}
