// Demo Zernike polynomials

#include <memory>
#include <cmath>
#include <format>

import sm.algo;
import sm.vvec;
import mplot.visual;
import mplot.graphvisual;

int main()
{
    mplot::Visual<> v (1024, 768, "Zernike Radial Polynomials (Rnm)");

    sm::vvec<double> rho;
    rho.linspace (0.001, 1, 200);

    mplot::DatasetStyle ds (mplot::stylepolicy::lines);

    // You can require that n - |m| is even to "ensure the rotational invariant property is met"
    // (see https://doi.org/10.7554/eLife.54026, p 18)
    constexpr bool require_n_minus_abs_m_even = false;

    for (unsigned int n = 0; n < 10; ++n) {
        sm::vec<float> offset = {1.4f * n, 2.4f, 0};
        sm::vec<float> offset2 = {1.4f * n, 0, 0};
        auto gv = std::make_unique<mplot::GraphVisual<double>> (offset);
        gv->set_parent (v.get_id());
        gv->xlabel = "rho";
        gv->ylabel = "Rnm";
        gv->setlimits (0.0, 1.0, -10.0, 10.0);

        auto gv2 = std::make_unique<mplot::GraphVisual<double>> (offset2);
        gv2->set_parent (v.get_id());
        gv2->xlabel = "rho";
        gv2->ylabel = "Rnm";
        gv2->setlimits (0.0, 1.0, -10.0, 10.0);

        bool have_data = false;

        for (int m = 0; m <= static_cast<int>(n); ++m) {

            if constexpr (require_n_minus_abs_m_even) {
                if ((n - std::abs(m)) % 2 != 0) { continue; }
            }

            sm::vvec<double> Rnm;
            sm::vvec<double> Rnm2;
            for (auto rh : rho) {
                Rnm.push_back (sm::algo::zern_radial_poly (n, m, rh));
                Rnm2.push_back (sm::algo::zern_radial_poly (n, -m, rh));
            }
            ds.datalabel = std::format ("n{}, m{}", n, m);
            ds.linecolour = mplot::DatasetStyle::datacolour (m + n);
            gv->setdata (rho, Rnm, ds);
            ds.datalabel = std::format ("n{}, m{}", n, -m);
            gv2->setdata (rho, Rnm2, ds);
            have_data = true;
        }

        if (have_data) {
            gv->finalize();
            gv2->finalize();
            v.addVisualModel (gv);
            v.addVisualModel (gv2);
        }
    }

    v.keepOpen();
}
