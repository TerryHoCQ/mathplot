// Demo Zernike polynomials

#include <format>
#include <complex>
#include <algorithm>
#include <sm/mathconst>
#include <sm/algo>
#include <sm/vvec>
#include <mplot/Visual.h>
#include <mplot/PolarVisual.h>

int main()
{
    // You can require that n - |m| is even to "ensure the rotational invariant property is met"
    // (see https://doi.org/10.7554/eLife.54026, p 18)
    constexpr bool require_n_minus_abs_m_even = false;

    // Choose flat or 3D plots
    constexpr bool flat_plots = true;

    using mc = sm::mathconst<double>;

    mplot::Visual<> v (1024, 768, "Zernike Polynomials");
    v.rotateAboutNearest (true);

    constexpr std::size_t N = 100;
    sm::vvec<double> rho;
    rho.linspace (0.0, 1.0, N);

    sm::vvec<double> theta;
    theta.linspace (0.0, mc::two_pi, N);

    // We impose a signal limit to avoid plotting infinitely high graphs
    constexpr double siglimit = 10.0;

    for (unsigned int n = 0; n <= 16; ++n) {

        for (int m = 0; m <= static_cast<int>(n); ++m) {

            if constexpr (require_n_minus_abs_m_even) { if ((n - std::abs(m)) % 2 != 0) { continue; } }

            auto pv = std::make_unique<mplot::PolarVisual<double>> (sm::vec<float>{1.25f * n, 1.25f * m, 0});
            v.bindmodel (pv);
            pv->cm.setType (mplot::ColourMapType::Cork);
            pv->setFrameColour (mplot::colour::goldenrod1);
            pv->setTextColour (mplot::colour::black);
            pv->radius = 0.5f;
            pv->tf.fontsize = 0.05f;
            pv->numrings = N;
            pv->numsegs = N;
            pv->addLabel (std::format ("n{}, m{}", n, m), sm::vec<float>{0.2,-0.58,0}, mplot::TextFeatures(0.08f));
            pv->twodimensional (false);

            sm::vvec<double> Vnm_real;
            for (auto rh : rho) {
                double r_nm = std::clamp (sm::algo::zern_radial_poly (n, m, rh), -siglimit, siglimit);
                for (auto th : theta) {
                    Vnm_real.push_back (std::real(sm::algo::zern_polynomial (m, r_nm, th)));
                }
            }

            // Replace NaN with 0
            for (auto& v : Vnm_real) { if (std::isnan (v)) { v = 0.0; } }

            pv->setScalarData (&Vnm_real);

            // Flat plot if necessary (above clamps and isnan testing should make this unnecessary)
            if constexpr (flat_plots) {
                pv->zScale.null_scaling();
            } else {
                pv->zScale.output_range = { -1.0f, 1.0f };
                pv->zScale.compute_scaling (-siglimit, siglimit);
            }

            pv->finalize();
            v.addVisualModel (pv);
        }
    }

    v.keepOpen();
}
