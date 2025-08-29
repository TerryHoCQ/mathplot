/*
 * Compute a single Izhikevich neuron model and plot. This computes the abcd/ABC form of the
 * equations as published in Izhikevich, Simple Model of Spiking Neurons, IEEE Transactions on
 * Neural Networks, Vol. 14. No. 6, 2003.
 */

#include <sm/vec>
#include <sm/vvec>
#include <sm/range>
#include <sm/grid>
#include <mplot/Visual.h>
#include <mplot/GraphVisual.h>
#include <sm/config>

// A simple Izhikevich neuron model class
struct izhi
{
    // state variables
    float I = 0.0f;      // Input current
    float u = -10.0f;    // 'The refractory variable'
    float v = -70.0f;    // 'Membrane voltage'

    // Parameters. In 'abc' model statement format.
    float a = 0.03f;
    float b = 0.193f;
    float c = -65.0f;
    float d = 0.05f;

    float A = 0.032f;
    float B = 4.0f;
    float C = 113.147f;

    float T = 0.4f;
    float SI = 5.0f;
    float vpeak = 30.0f;

    // vdot and udot computations
    float dv (const float _u, const float _v) { return (A*T) * _v * _v + (B*T) * _v + (C*T) - _u * T + I * (T/SI); }
    float du (const float _u, const float _v) { return a * T * (b * _v - _u); }

    // Apply one timestep of the differential equations for the model
    void step()
    {
        bool spike = (v > vpeak);       // This is the reset condition
        float _du = this->du (u, v);    // Compute now as in next line, v may change
        v = spike ? c       : (v + this->dv (u, v));
        u = spike ? (u + d) : (u + _du);
    }

    // Compute nullclines. For Vn, the given input membrane voltages, return u and v nullclines in u_nc and v_nc
    void nullclines (const sm::vvec<float>& Vn, sm::vvec<float>& u_nc, sm::vvec<float>& v_nc)
    {
        u_nc.resize (Vn.size(), 0.0f);
        v_nc.resize (Vn.size(), 0.0f);
        for (unsigned int i = 0; i < Vn.size(); ++i) {
            v_nc[i] = A * Vn[i] * Vn[i] + B * Vn[i] + C + I / SI;
            u_nc[i] = Vn[i] * b;
        }
    }

    // Compute the vectorfield of du and dv vs. u and v
    void vectorfield (const sm::vvec<float>& _u, const sm::vvec<float>& _v, sm::vvec<sm::vec<float, 2>>& vecfield)
    {
        if (_u.size() != _v.size()) { return; }
        vecfield.resize (_u.size() * _v.size(), {0,0});
        for (unsigned int j = 0; j < _u.size(); ++j) {
            unsigned int shft = j * _v.size();
            for (unsigned int i = 0; i < _v.size(); ++i) {
                vecfield[i + shft] = { this->dv (_u[j], _v[i]), this->du (_u[j], _v[i]) };
            }
        }
    }
};


int main()
{
    constexpr unsigned int N = 1000; // How many steps to run
    constexpr bool twodee = false;   // Should graphs be 2D only or rotatable?

    /*
     * Perform simulation
     */

    // Create class and vars
    sm::vvec<float> u(N, 0.0f);
    sm::vvec<float> v(N, 0.0f);
    izhi iz;

    // Set izhi params from config
    std::string jsonfile ("../examples/izhikevich.json");
    sm::config config(jsonfile);
    if (config.ready) {
        // Parameters
        iz.a = config.getFloat ("a", iz.a);
        iz.b = config.getFloat ("b", iz.b);
        iz.c = config.getFloat ("c", iz.c);
        iz.d = config.getFloat ("d", iz.d);
        iz.A = config.getFloat ("A", iz.A);
        iz.B = config.getFloat ("B", iz.B);
        iz.C = config.getFloat ("C", iz.C);
        iz.T = config.getFloat ("T", iz.C);
        iz.SI = config.getFloat ("SI", iz.SI);
        iz.vpeak = config.getFloat ("vpeak", iz.vpeak);
        // Initial values of state vars
        iz.u = config.getFloat ("u0", iz.u);
        iz.v = config.getFloat ("v0", iz.v);
    }

    // Run sim
    for (unsigned int i = 0; i < N; ++i) {
        iz.step();
        v[i] = iz.v;
        u[i] = iz.u;
    }
    // Find range of the state variables u and v for plotting
    sm::range<float> v_range = v.range();
    if (v_range.max > iz.vpeak) { v_range.max = iz.vpeak; }
    sm::range<float> u_range = u.range();

    /*
     * Compute nullclines
     */

    sm::vvec<float> u_nc;
    sm::vvec<float> v_nc;
    sm::vvec<float> vrng;
    vrng.linspace (-80.0f, -20.0f, 1000);
    iz.nullclines (vrng, u_nc, v_nc);

    /*
     * Compute du/dv vector field
     */

    static constexpr size_t qN = 50;
    sm::vvec<float> qurng; // y axis
    sm::vvec<float> qvrng; // x axis
    qvrng.linspace (v_range.min, v_range.max, qN);
    qurng.linspace (u_range.min, u_range.max, qN);
    sm::vvec<sm::vec<float, 2>> du_dv_vecfield;
    iz.vectorfield (qurng, qvrng, du_dv_vecfield);
    // Now plot with a Grid and a GraphVisual? Or initially with a QuiverVisual
    sm::vec<float, 2> gridspacing = {
        (v_range.span()) / (qN-1),
        (u_range.span()) / (qN-1)
    };
    sm::vec<float, 2> gridzero = { v_range.min, u_range.min };
    sm::grid<unsigned int, float> grid (qN, qN, gridspacing, gridzero);

    /*
     * Visualize results
     */

    mplot::Visual vis(1280, 768, "Izhikevich Neuron Model");
    vis.setSceneTrans (sm::vec<float,3>({-0.877793f, -0.281277f, -3.9f}));
    vis.lightingEffects();

    // Time
    sm::vvec<float> t(N, 0.0f);
    t.linspace (0.0f, N/100.0f, N);

    // Set default dataset graphing styles
    mplot::DatasetStyle ds;
    ds.linewidth = 0.003f;
    ds.linecolour = mplot::colour::grey30;
    ds.markersize = 0.015f;
    ds.markerstyle = mplot::markerstyle::uphexagon;

    // Graph membrane voltage vs. time
    auto gv = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>({-0.5,-0.5,0}));
    vis.bindmodel (gv);
    gv->twodimensional (twodee);
    gv->setsize (1,0.8);
    gv->xlabel = "t";
    gv->ylabel = "v";
    ds.datalabel = "v(t)";
    gv->setdata (t, v, ds);
    gv->finalize();
    vis.addVisualModel (gv);

    // Graph u(t)
    auto gu = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>({-0.5,0.6,0}));
    vis.bindmodel (gu);
    gu->twodimensional (twodee);
    gu->setsize (1,0.5);
    gu->xlabel = "t";
    gu->ylabel = "u";
    ds.datalabel = "u(t)";
    ds.markercolour = mplot::colour::crimson;
    gu->setdata (t, u, ds);
    gu->finalize();
    vis.addVisualModel (gu);

    // Graph nullclines, u vs v and vector field
    ds.showlines = false;
    auto gp = std::make_unique<mplot::GraphVisual<float>> (sm::vec<float>({0.9,-0.5,0}));
    vis.bindmodel (gp);
    gp->twodimensional (twodee);
    gp->setsize (1.6,1.6);

    // Fix plotting range
    gp->setlimits_x (v_range);
    gp->setlimits_y (u_range);

    gp->xlabel = "v";
    gp->ylabel = "u";

    // nullcline for the u variable
    ds.markercolour = mplot::colour::crimson;
    ds.datalabel = "u nc";
    gp->setdata (vrng, u_nc, ds);

    // nullcline for the v variable
    ds.markercolour = mplot::colour::royalblue;
    ds.datalabel = "v nc";
    gp->setdata (vrng, v_nc, ds);

    // The evolution of v and u wrt time
    ds.markercolour = mplot::colour::black;
    ds.datalabel = "u(v)";
    gp->setdata (v, u, ds);

    // Plot quivs within graphvisual
    ds.datalabel = "quivs";
    // Set a linear gain to apply to the quivers...
    ds.quiver_gain = { 0.01f, 0.1f, 1.0f };
    // ...and then if the lengths should be log-scaled, call quiver_setlog()
    gp->quiver_setlog();
    ds.quiver_colourmap.setType (mplot::ColourMapType::Batlow);
    ds.quiver_conewidth = 1.8f;
    ds.quiver_arrowhead_prop = 0.35f;
    ds.quiver_thickness_gain = 1.5f;
    ds.markerstyle = mplot::markerstyle::quiver;
    //ds.quiver_flagset.set (static_cast<unsigned int>(mplot::quiver_flags::show_zeros));
    //ds.quiver_flagset.set (static_cast<unsigned int>(mplot::quiver_flags::marker_sphere));
    gp->setdata (grid, du_dv_vecfield, ds);
    gp->finalize();
    vis.addVisualModel (gp);

    // Keep showing graphs until user exits
    vis.keepOpen();

    return 0;
}
