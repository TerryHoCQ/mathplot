/*
 * A simple polar plot example.
 */

#include <sm/mathconst>
#include <sm/vvec>
#include <mplot/Visual.h>
#include <mplot/PolarVisual.h>

int main()
{
    using mc = sm::mathconst<double>;

    mplot::Visual<> v (1024, 768, "Polar plot");

    constexpr std::size_t N = 40;
    sm::vvec<double> rho;
    rho.linspace (0.0, 1.0, N);
    sm::vvec<double> theta;
    theta.linspace (0.0, mc::two_pi, N);

    auto pv = std::make_unique<mplot::PolarVisual<double>> (sm::vec<float>{0.0f});
    v.bindmodel (pv);
    pv->cm.setType (mplot::ColourMapType::Cork);
    pv->setFrameColour (mplot::colour::black);
    pv->setTextColour (mplot::colour::black);
    pv->radius = 1.0f;
    pv->tf.fontsize = 0.08f;
    pv->ticklabelgap = 0.05f;
    pv->numrings = N;
    pv->numsegs = N;
    pv->twodimensional (false);

    // Make a function that increases radially near theta == 0 and theta == pi/2 and is 0 everywhere else
    sm::vvec<double> data;
    for (auto rh : rho) {
        for (auto th : theta) {
            if (std::abs (th) < 0.1 || std::abs (th - mc::pi_over_2) < 0.1) {
                data.push_back (rh);
            } else {
                data.push_back (0.0);
            }
        }
    }

    pv->setScalarData (&data);

    pv->zScale.output_range = {-1.0f, 1.0f};
    pv->zScale.compute_scaling (-2, 2);

    pv->finalize();
    v.addVisualModel (pv);

    v.keepOpen();
}
