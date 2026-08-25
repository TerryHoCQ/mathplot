/*
 * Test Simulated Annealing algorithm on the Rosenbrock banana function.
 */

#include <memory>
#include <iostream>
#include <chrono>

import sm.vec;
import sm.vvec;
import sm.hexgrid;
import sm.anneal;

import mplot.visual;
import mplot.triframevisual;
import mplot.hexgridvisual;
import mplot.polygonvisual;

// Here's the Rosenbrock banana function
FLT banana (sm::vvec<FLT> xy) {
    FLT a = 1.0;
    FLT b = 100.0;
    FLT x = xy[0];
    FLT y = xy[1];
    FLT rtn = ((a-x)*(a-x)) + (b * (y-(x*x)) * (y-(x*x)));
   return rtn;
}

int main()
{
    // Check banana function
    FLT test = banana ({1.0, 1.0});
    std::cout << "test point on banana function = " << test << " (should be 0).\n";

    // Initial point and ranges
    sm::vvec<FLT> p = { 0.5, -0.5};
    std::cout << "Start point on banana function = " << banana(p) << ".\n";
    sm::vvec<sm::vec<FLT,2>> p_rng = {{ {-1.1, 1.1}, {-1.1, 1.1} }};

#ifdef VISUALISE
    // Set up a visual environment
    mplot::Visual v(2600, 1800, "Rosenbrock bananas");
    v.zNear = 0.001;
    v.zFar = 100000;
    v.fov=60;
    v.showCoordArrows (true);
    v.lightingEffects (true);

    sm::vec<float> offset = {0,0,0};
    sm::hexgrid<float> hg (0.01, 10, 0);
    hg.set_circular_boundary (2.5);
    std::vector<FLT> banana_vals(hg.num(), 0.0f);
    for (size_t i = 0; i < hg.num(); ++i) {
        banana_vals[i] = banana ({hg.d_x[i], hg.d_y[i]});
    }
    sm::interval<FLT> mm = sm::interval<FLT>::get_from (banana_vals);
    std::cout << "Banana surface range: " << mm << std::endl;
    auto hgv = std::make_unique<mplot::HexGridVisual<FLT>>(&hg, offset);
    hgv->set_parent (v.get_id());
    hgv->hexVisMode = mplot::HexVisMode::Triangles;
    hgv->cm.setType (mplot::ColourMapType::Viridis);
    hgv->setScalarData (&banana_vals);
    hgv->zScale.set_params (0.001f, 0.0f);
    hgv->colourScale.compute_scaling (0.01f, 5.0f);
    hgv->setAlpha (0.4f);
    hgv->finalize();
    v.addVisualModel (hgv);

    sm::vec<float, 3> polypos = { static_cast<float>(p[0]), static_cast<float>(p[1]), 0.0f };

    // One object for the 'candidate' position
    std::array<float, 3> col = { 0, 1, 0 };
    auto candup = std::make_unique<mplot::PolygonVisual<>>(offset, polypos, sm::vec<float>({1,0,0}), 0.005f, 0.4f, col, 20);
    candup->set_parent (v.get_id());
    candup->finalize();

    // A second object for the 'best' position
    col = { 1, 0, 0 };
    auto bestup = std::make_unique<mplot::PolygonVisual<>>(offset, polypos, sm::vec<float>({1,0,0}), 0.001f, 0.8f, col, 10);
    bestup->set_parent (v.get_id());
    bestup->finalize();

    // A third object for the currently accepted position
    col = { 1, 0, 0.7f };
    auto currup = std::make_unique<mplot::PolygonVisual<>> (offset, polypos, sm::vec<float>({1,0,0}), 0.005f, 0.6f, col, 20);
    currup->set_parent (v.get_id());
    currup->finalize();

    auto candp = v.addVisualModel (candup);
    auto bestp = v.addVisualModel (bestup);
    auto currp = v.addVisualModel (currup);
#endif

    sm::anneal<FLT> anneal(p, p_rng);

    anneal.temperature_ratio_scale = FLT{1e-3};
    anneal.temperature_anneal_scale = FLT{200};
    anneal.cost_parameter_scale_ratio = FLT{1.5};
    anneal.acc_gen_reanneal_ratio = FLT{1e-3};
    anneal.delta_param = FLT{0.01};
    anneal.f_x_best_repeat_max = 15;
    anneal.enable_reanneal = false;
    anneal.reanneal_after_steps = 100;

    anneal.init();

    // Now do the business
    while (anneal.state != sm::anneal_state::ready_to_stop) {

        // ...and on each loop, compute the objectives that anneal asks you to:
        if (anneal.state == sm::anneal_state::need_to_compute) {
            // Compute the candidate objective value
            anneal.f_x_cand = banana (anneal.x_cand);

        } else if (anneal.state == sm::anneal_state::need_to_compute_set) {
            // Compute objective values for reannealing
            anneal.f_x_plusdelta = banana (anneal.x_plusdelta);
            //anneal.f_x = banana (anneal.x); // no need

        } else {
            throw std::runtime_error ("Unexpected state for anneal object.");
        }

#ifdef VISUALISE
        // You can update the visualisation within this loop if you like:
        candp->position = { static_cast<float>(anneal.x_cand[0]),
                            static_cast<float>(anneal.x_cand[1]),
                            static_cast<float>(anneal.f_x_cand - FLT{0.15}) };
        candp->reinit();
        bestp->position = { static_cast<float>(anneal.x_best[0]),
                            static_cast<float>(anneal.x_best[1]),
                            static_cast<float>(anneal.f_x_best - FLT{0.15}) };
        bestp->reinit();
        currp->position = { static_cast<float>(anneal.x[0]),
                            static_cast<float>(anneal.x[1]),
                            static_cast<float>(anneal.f_x - FLT{0.15}) };
        currp->reinit();
        v.waitevents (0.0166);
        v.render();
#endif
        anneal.step();
        //if (anneal.steps > 10) { anneal.state = sm::anneal_state::ready_to_stop; }
    }

#ifdef VISUALISE
    std::cout << "Last anneal stats: num_improved " << anneal.num_improved << ", num_worse: " << anneal.num_worse
              << ", num_worse_accepted: " << anneal.num_worse_accepted << " (as proportion: "
              << ((double)anneal.num_worse_accepted/(double)anneal.num_worse) << ")\n\n";

    std::cout << "FINISHED in " << anneal.steps << " calls to Anneal::step().\n"
              << "Best parameters: " << anneal.x_best << "\n"
              << "Best params obj: " << anneal.f_x_best
              << " vs. [Whatever it is], the true obj_f.min().\n";

    std::cout << "(You can close the window with 'Ctrl-q' or take a snapshot with 'Ctrl-s'. 'Ctrl-h' for other help).\n";

    v.keepOpen();
#else
    std::cout << anneal.steps << "," << anneal.f_x_best << "\n";
#endif

    return 0;
}
