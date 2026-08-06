/*
 * Visualize sm::rungekutta4, the 4th order Runge-Kutta ODE solver.
 *
 * Four systems are integrated, each using a different dependent variable type, to show that the
 * same rungekutta4 code handles a single scalar ODE, a fixed-size system of ODEs (sm::vec<T, N>)
 * and a system of an arbitrary number of ODEs (sm::vvec<T>). The fourth system is a classic
 * stiff linear system, included to show how a widely separated pair of eigenvalues challenges an
 * explicit method like RK4.
 *
 * For each system, two graphs are shown: the numerical (RK4) solution overlaid with the known
 * analytic solution, and the delta between the two at every timestep. Axis limits are set manually
 * from the union of all series plotted on a graph, because GraphVisual only auto-scales an axis
 * from the first dataset added to it.
 */
#include <memory>
#include <iostream>
#include <cmath>
#include <cstdint>
#include <string>
#include <algorithm>
#include <vector>

import mplot.visual;
import mplot.graphvisual; // exports sm.vvec and sm.vec
import mplot.unicode;
import sm.rungekutta4;

int main()
{
    int rtn = 0;

    try {
        constexpr double h = 0.001;
        constexpr std::uint32_t n_steps = 1000;

        // Independent variable, t, common to all three systems
        sm::vvec<double> t (n_steps + 1);
        for (std::uint32_t i = 0; i <= n_steps; ++i) { t[i] = static_cast<double>(i) * h; }

        mplot::Visual v(1600, 1200, "sm::rungekutta4 examples");
        v.backgroundWhite();

        constexpr float col_step = 1.8f;
        constexpr float row_step = 1.6f;

        // Zoom/pan out so that all 4 rows x 2 columns of graphs are in view
        v.setSceneTrans (sm::vec<float,3>{ float{-3.16188}, float{1.03305}, float{-10.4181} });
        v.setSceneRotation (sm::quaternion<float>{ float{1}, float{0}, float{0}, float{0} });


        mplot::ColourMap<float> num_cm (mplot::ColourMapType::Oslo);
        mplot::ColourMap<float> ana_cm (mplot::ColourMapType::Bamako);
        mplot::ColourMap<float> delt_cm (mplot::ColourMapType::Lajolla);

        mplot::DatasetStyle ds_numeric (mplot::stylepolicy::markers);
        ds_numeric.datalabel = "rk4";
        ds_numeric.markercolour = num_cm.convert (0.4f);
        ds_numeric.markersize *= 0.6f;
        ds_numeric.markerstyle = mplot::markerstyle::circle;

        mplot::DatasetStyle ds_analytic (mplot::stylepolicy::markers);
        ds_analytic.datalabel = "analytic";
        ds_analytic.markercolour = ana_cm.convert (0.4f);
        ds_analytic.markerstyle = mplot::markerstyle::circle;

        mplot::DatasetStyle ds_delta (mplot::stylepolicy::markers);
        ds_delta.datalabel = mplot::unicode::toUtf8 (mplot::unicode::Delta);
        ds_delta.markercolour = delt_cm.convert (0.4f);
        ds_delta.markersize *= 0.6f;
        ds_delta.markerstyle = mplot::markerstyle::circle;

        // 1) A single scalar ODE: dx/dt = -x, x(0) = 1. Analytic solution: x(t) = exp(-t)
        {
            sm::rungekutta4<double> rk ([](const double&, const double& x) { return -x; }, 1.0, 0.0, h);
            sm::vvec<double> numerical = rk.integrate (n_steps);
            sm::vvec<double> analytic = (t * -1.0).exp();
            sm::vvec<double> delta = numerical - analytic;

            auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{0, 0, 0});
            gv->set_parent (v.get_id());
            gv->setlimits (t.min(), t.max(), std::min (numerical.min(), analytic.min()), std::max (numerical.max(), analytic.max()));
            gv->setdata (t, numerical, ds_numeric);
            ds_analytic.datalabel = "analytic: x(t) = exp(-t)";
            gv->setdata (t, analytic, ds_analytic);
            gv->xlabel = "t";
            gv->ylabel = "x";
            gv->addLabel ("Scalar ODE: dx/dt = -x, x(0) = 1", sm::vec<float>{0.8, 1.3}, mplot::TextFeatures(0.09f));
            gv->finalize();
            v.addVisualModel (gv);

            auto gvd = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{col_step, 0, 0});
            gvd->set_parent (v.get_id());
            gvd->setlimits (t.min(), t.max(), delta.min(), delta.max());
            ds_delta.datalabel = mplot::unicode::toUtf8(mplot::unicode::Delta) + "x";
            gvd->setdata (t, delta, ds_delta);
            gvd->xlabel = "t";
            gvd->ylabel = "numerical - analytic";
            gvd->finalize();
            v.addVisualModel (gvd);
        }

        // 2) A fixed-size system of 2 ODEs (simple harmonic motion): dx/dt = v, dv/dt = -x,
        // x(0) = 1, v(0) = 0. Analytic solution: x(t) = cos(t), v(t) = -sin(t)
        {
            using State = sm::vec<double, 2>;
            sm::rungekutta4<double, State> rk (
                [](const double&, const State& x) { return State{ x[1], -x[0] }; },
                State{ 1.0, 0.0 }, 0.0, h);
            sm::vvec<State> numerical = rk.integrate (n_steps);

            sm::vvec<State> analytic (n_steps + 1);
            for (std::uint32_t i = 0; i <= n_steps; ++i) {
                analytic[i] = State{ std::cos (t[i]), -std::sin (t[i]) };
            }

            const std::string labels[2] = { "x", "v" };

            // Extract each component into its own vvec<double>, and the deltas between them
            std::vector<sm::vvec<double>> num_c (2, sm::vvec<double>(n_steps + 1));
            std::vector<sm::vvec<double>> an_c (2, sm::vvec<double>(n_steps + 1));
            std::vector<sm::vvec<double>> delta_c (2);
            for (unsigned int c = 0; c < 2; ++c) {
                for (std::uint32_t i = 0; i <= n_steps; ++i) {
                    num_c[c][i] = numerical[i][c];
                    an_c[c][i] = analytic[i][c];
                }
                delta_c[c] = num_c[c] - an_c[c];
            }

            // GraphVisual only auto-scales its y axis from the first dataset added, so
            // compute the union range across all components before calling setdata
            double ymin = std::min ({ num_c[0].min(), num_c[1].min(), an_c[0].min(), an_c[1].min() });
            double ymax = std::max ({ num_c[0].max(), num_c[1].max(), an_c[0].max(), an_c[1].max() });
            double dmin = std::min (delta_c[0].min(), delta_c[1].min());
            double dmax = std::max (delta_c[0].max(), delta_c[1].max());

            auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>({0, -row_step, 0}));
            gv->set_parent (v.get_id());
            gv->setlimits (t.min(), t.max(), ymin, ymax);
            gv->addLabel ("Simple harmonic motion: dx/dt = v, dv/dt = -x", sm::vec<float>{0.7, 1.3}, mplot::TextFeatures(0.09f));

            auto gvd = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{col_step, -row_step, 0});
            gvd->set_parent (v.get_id());
            gvd->setlimits (t.min(), t.max(), dmin, dmax);

            for (unsigned int c = 0; c < 2; ++c) {
                ds_numeric.datalabel = "rk4 " + labels[c];
                ds_numeric.markercolour = num_cm.convert (0.1f * (c + 4));
                gv->setdata (t, num_c[c], ds_numeric);
                ds_analytic.datalabel = "analytic " + labels[c];
                ds_analytic.markercolour = ana_cm.convert (0.1f * (c + 4));
                gv->setdata (t, an_c[c], ds_analytic);
                ds_delta.datalabel = mplot::unicode::toUtf8(mplot::unicode::Delta) + labels[c];
                ds_delta.markercolour = delt_cm.convert (0.1f * (c + 4));
                gvd->setdata (t, delta_c[c], ds_delta);
            }

            gv->xlabel = "t";
            gv->ylabel = "x, v";
            gv->finalize();
            v.addVisualModel (gv);

            gvd->xlabel = "t";
            gvd->ylabel = "numerical - analytic";
            gvd->finalize();
            v.addVisualModel (gvd);
        }

        // 3) A system of an arbitrary number of ODEs: 5 independent exponential decays,
        // each with its own rate. dx_i/dt = -rate_i * x_i, x_i(0) = 1. Analytic solution:
        // x_i(t) = exp(-rate_i * t)
        {
            using State = sm::vvec<double>;
            State rate = { 0.1, 0.5, 1.0, 2.0, 3.0 };
            sm::rungekutta4<double, State> rk (
                [rate](const double&, const State& x) { return x * rate * -1.0; },
                State{ 1.0, 1.0, 1.0, 1.0, 1.0 }, 0.0, h);
            sm::vvec<State> numerical = rk.integrate (n_steps);

            sm::vvec<State> analytic (n_steps + 1);
            for (std::uint32_t i = 0; i <= n_steps; ++i) {
                analytic[i] = (rate * -t[i]).exp();
            }

            const unsigned int n_comp = static_cast<unsigned int>(rate.size());
            std::vector<sm::vvec<double>> num_c (n_comp, sm::vvec<double>(n_steps + 1));
            std::vector<sm::vvec<double>> an_c (n_comp, sm::vvec<double>(n_steps + 1));
            std::vector<sm::vvec<double>> delta_c (n_comp);
            for (unsigned int c = 0; c < n_comp; ++c) {
                for (std::uint32_t i = 0; i <= n_steps; ++i) {
                    num_c[c][i] = numerical[i][c];
                    an_c[c][i] = analytic[i][c];
                }
                delta_c[c] = num_c[c] - an_c[c];
            }

            double ymin = num_c[0].min();
            double ymax = num_c[0].max();
            double dmin = delta_c[0].min();
            double dmax = delta_c[0].max();
            for (unsigned int c = 0; c < n_comp; ++c) {
                ymin = std::min ({ ymin, num_c[c].min(), an_c[c].min() });
                ymax = std::max ({ ymax, num_c[c].max(), an_c[c].max() });
                dmin = std::min (dmin, delta_c[c].min());
                dmax = std::max (dmax, delta_c[c].max());
            }

            auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{2 * col_step + 0, 0, 0});
            gv->set_parent (v.get_id());
            gv->setlimits (t.min(), t.max(), ymin, ymax);
            gv->addLabel ("Five exponential decays", sm::vec<float>{1.2, 1.3}, mplot::TextFeatures(0.09f));

            auto gvd = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{2 * col_step + col_step, 0, 0});
            gvd->set_parent (v.get_id());
            gvd->setlimits (t.min(), t.max(), dmin, dmax);

            for (unsigned int c = 0; c < n_comp; ++c) {
                ds_numeric.datalabel = "rk4[" + std::to_string (c) + "]";
                ds_numeric.markercolour = num_cm.convert (0.1f * (c + 4));
                gv->setdata (t, num_c[c], ds_numeric);

                ds_analytic.datalabel = "analytic[" + std::to_string (c) + "]";
                ds_analytic.markercolour = ana_cm.convert (0.1f * (c + 4));
                gv->setdata (t, an_c[c], ds_analytic);

                ds_delta.datalabel = mplot::unicode::toUtf8(mplot::unicode::Delta) + "[" + std::to_string (c) + "]";
                ds_delta.markercolour = delt_cm.convert (0.1f * (c + 4));
                gvd->setdata (t, delta_c[c], ds_delta);
            }

            gv->xlabel = "t";
            gv->ylabel = "x_i";
            gv->finalize();
            v.addVisualModel (gv);

            gvd->xlabel = "t";
            gvd->ylabel = "numerical - analytic";
            gvd->finalize();
            v.addVisualModel (gvd);
        }

        // 4) A stiff linear system of 2 ODEs: dx/dt = 998x - 1998y, dy/dt = 1000x - 2000y,
        // x(0) = 1, y(0) = 2. The system matrix has eigenvalues -2 and -1000, so the fast
        // mode decays 500x quicker than the slow mode: a classic test of stiffness, since
        // an explicit method must take steps small enough to remain stable for the fast
        // mode even though it becomes negligible almost immediately. Analytic solution
        // (from diagonalising the system): x(t) = (-999 exp(-2t) + 1498 exp(-1000t)) / 499,
        // y(t) = (-500 exp(-2t) + 1498 exp(-1000t)) / 499
        {
            using State = sm::vec<double, 2>;
            sm::rungekutta4<double, State> rk (
                [](const double&, const State& s) {
                    return State{ 998.0 * s[0] - 1998.0 * s[1], 1000.0 * s[0] - 2000.0 * s[1] };
                },
                State{ 1.0, 2.0 }, 0.0, h);
            sm::vvec<State> numerical = rk.integrate (n_steps);

            sm::vvec<State> analytic (n_steps + 1);
            for (std::uint32_t i = 0; i <= n_steps; ++i) {
                double e_slow = std::exp (-2.0 * t[i]);
                double e_fast = std::exp (-1000.0 * t[i]);
                analytic[i] = State{ (-999.0 * e_slow + 1498.0 * e_fast) / 499.0,
                                      (-500.0 * e_slow + 1498.0 * e_fast) / 499.0 };
            }

            const std::string labels[2] = { "x", "y" };

            std::vector<sm::vvec<double>> num_c (2, sm::vvec<double>(n_steps + 1));
            std::vector<sm::vvec<double>> an_c (2, sm::vvec<double>(n_steps + 1));
            std::vector<sm::vvec<double>> delta_c (2);
            for (unsigned int c = 0; c < 2; ++c) {
                for (std::uint32_t i = 0; i <= n_steps; ++i) {
                    num_c[c][i] = numerical[i][c];
                    an_c[c][i] = analytic[i][c];
                }
                delta_c[c] = num_c[c] - an_c[c];
            }

            double ymin = std::min ({ num_c[0].min(), num_c[1].min(), an_c[0].min(), an_c[1].min() });
            double ymax = std::max ({ num_c[0].max(), num_c[1].max(), an_c[0].max(), an_c[1].max() });
            double dmin = std::min (delta_c[0].min(), delta_c[1].min());
            double dmax = std::max (delta_c[0].max(), delta_c[1].max());

            auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{2 * col_step, -row_step, 0});
            gv->set_parent (v.get_id());
            gv->setlimits (t.min(), t.max(), ymin, ymax);
            gv->addLabel ("Stiff linear system:\ndx/dt = 998x - 1998y, dy/dt = 1000x - 2000y", sm::vec<float>{0.8, 1.3}, mplot::TextFeatures(0.09f));

            auto gvd = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float>{2 * col_step + col_step, -row_step, 0});
            gvd->set_parent (v.get_id());
            gvd->setlimits (t.min(), t.max(), dmin, dmax);

            for (unsigned int c = 0; c < 2; ++c) {
                ds_numeric.datalabel = "rk4 " + labels[c];
                ds_numeric.markercolour = num_cm.convert (0.1f * (c + 4));
                gv->setdata (t, num_c[c], ds_numeric);
                ds_analytic.datalabel = "analytic " + labels[c];
                ds_analytic.markercolour = ana_cm.convert (0.1f * (c + 4));
                gv->setdata (t, an_c[c], ds_analytic);
                ds_delta.datalabel = mplot::unicode::toUtf8(mplot::unicode::Delta) + labels[c];
                ds_delta.markercolour = delt_cm.convert (0.1f * (c + 4));
                gvd->setdata (t, delta_c[c], ds_delta);
            }

            gv->xlabel = "t";
            gv->ylabel = "x, y";
            gv->finalize();
            v.addVisualModel (gv);

            gvd->xlabel = "t";
            gvd->ylabel = "numerical - analytic";
            gvd->finalize();
            v.addVisualModel (gvd);
        }

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
