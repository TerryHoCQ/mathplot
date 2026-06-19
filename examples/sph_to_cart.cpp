// Visualize spherical to cartesian conversion in sm::vec
#include <memory>
#include <vector>

import sm.vec;
import mplot.visual;
import mplot.quivervisual;

int main()
{
    using mc = sm::mathconst<float>;
    constexpr int n = 10;
    mplot::Visual<> v (1024, 768, "Spherical to cartesian");
    std::vector<sm::vec<float, 3>> coords(n * n);
    std::vector<sm::vec<float, 3>> quivs(n * n);
    std::vector<float> azim (n * n);

    float r = 2.0f;

    // Loops to create spherical coordinates in a regular manner
    for (int az = 0; az < n; ++az) {

        for (int th = 0; th < n; ++th) {
            sm::vec<float> sc = { r, (mc::two_pi / n) * th, (mc::pi / n) * az };
            coords[az * n + th] = sc.spherical_to_cartesian(); // Here's our use of the conversion
            quivs[az * n + th] = coords[az * n + th];
            quivs[az * n + th].renormalize();
            // azim is used to colour the quivers from the azimuthal angle
            azim[az * n + th] = static_cast<float>(az) / n;
        }
    }

    sm::vec<float, 3> offset = {};
    auto vmp = std::make_unique<mplot::QuiverVisual<float>>(&coords, offset, &quivs, mplot::ColourMapType::Jet);
    vmp->set_parent (v.get_id());
    vmp->setScalarData (&azim);
    vmp->fixed_length = 0.25f;
    vmp->fixed_quiver_thickness = 0.01f;
    vmp->finalize();
    v.addVisualModel (vmp);

    v.keepOpen();
}
