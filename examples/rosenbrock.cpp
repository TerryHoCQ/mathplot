/*
 * Test Nelder Mead Simplex algorithm on the Rosenbrock banana function.
 */

#include <sm/hexgrid>
#include <sm/nm_simplex>
#include <sm/vec>
#include <sm/vvec>
#include <sm/random>

#include <mplot/Visual.h>
#include <mplot/TriFrameVisual.h>
#include <mplot/HexGridVisual.h>
#include <iostream>
#include <chrono>

// Here's the Rosenbrock banana function
template<typename T>
T banana (T x, T y)
{
    constexpr T a = T{1};
    constexpr T b = T{100};
    T rtn = ((a - x) * (a - x)) + (b * (y - (x * x)) * (y - (x * x)));
    return rtn;
}

int main()
{
    int rtn = -1;

    // Set up a visual environment
    mplot::Visual v(2600, 1800, "Rosenbrock bananas");
    v.zNear = 0.001;
    v.zFar = 100000;
    v.fov = 60;
    v.showCoordArrows (true);
    v.lightingEffects (true);

    // Initialise the vertices
    sm::rand_uniform<FLT> rng(-3, 3);
    sm::vvec<FLT> v1 = { rng.get(), rng.get() };
    sm::vvec<FLT> v2 = { rng.get(), rng.get() };
    sm::vvec<FLT> v3 = { rng.get(), rng.get() };
    sm::vvec<sm::vvec<FLT>> i_vertices = { v1, v2, v3 };

    // Add a 'triangle visual' to be visualised as three rods
    sm::vec<float> _offset = {0,0,0};
    auto tfv = std::make_unique<mplot::TriFrameVisual<FLT>>(_offset);
    v.bindmodel (tfv);
    tfv->radius = 0.01f;
    tfv->sradius = 0.01f;
    std::vector<FLT> tri_values(3, 0);
    std::vector<sm::vec<float>> tri_coords(3);
    tri_coords[0] = { v1[0], v1[1], 0.0 };
    tri_coords[1] = { v2[0], v2[1], 0.0 };
    tri_coords[2] = { v3[0], v3[1], 0.0 };
    tfv->setScalarData (&tri_values);
    tfv->setDataCoords (&tri_coords);
    tfv->cm.setType (mplot::ColourMapType::Cividis);
    tfv->finalize();
    auto tfvp = v.addVisualModel (tfv);

    // Check banana function
    FLT test = banana<FLT> (1.0, 1.0);
    std::cout << "test point on banana function = " << test << " (should be 0).\n";

    // Evaluate banana function and plot
    sm::hexgrid hg (0.01, 10, 0);
    hg.setCircularBoundary (2.5);
    std::vector<FLT> banana_vals(hg.num(), 0.0f);
    for (size_t i = 0; i < hg.num(); ++i) {
        banana_vals[i] = banana<FLT> (hg.d_x[i], hg.d_y[i]);
    }
    sm::range<FLT> mm = sm::range<FLT>::get_from (banana_vals);
    std::cout << "Banana surface range: " << mm << std::endl;
    auto hgv = std::make_unique<mplot::HexGridVisual<FLT>>(&hg, _offset);
    v.bindmodel (hgv);
    hgv->hexVisMode = mplot::HexVisMode::Triangles;
    hgv->cm.setType (mplot::ColourMapType::Viridis);
    hgv->setScalarData (&banana_vals);
    hgv->zScale.setParams (0.001f, 0.0f);
    hgv->colourScale.compute_scaling (0.01f, 5.0f);
    hgv->setAlpha (0.4f);
    hgv->finalize();
    v.addVisualModel (hgv);

    sm::nm_simplex<FLT> simp(i_vertices);

    // The smaller you make the threshold, the nearer the algo will get
    simp.termination_threshold = std::numeric_limits<FLT>::epsilon();
    // You can prevent the algo getting stuck if termination_threshold is too small
    simp.too_many_operations = 10000;
    simp.objective = [](sm::vvec<FLT> x) { return banana<FLT>(x[0], x[1]); }; // objective defined as lambda

    // This is the same as the NM_Simplex::run function, but it is reproduced here to *visualize*
    // the Simplex as it descends the surface. For a more compact way to write your NM_Simplex, see
    // tests/testNMSimplex.cpp
    while (!v.readyToFinish()) {

        simp.reset (i_vertices);
        // if using plotting, then set up the render clock
        std::chrono::steady_clock::time_point lastrender = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point lastoptstep = std::chrono::steady_clock::now();

        // Now step until the algorithm is ready to finish
        while (simp.state != sm::nm_simplex_state::ReadyToStop && !v.readyToFinish()) {

            // Perform optimisation steps slowly
            std::chrono::steady_clock::duration sinceoptstep = std::chrono::steady_clock::now() - lastoptstep;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(sinceoptstep).count() > 50) {
                // Step the NM simplex optimizer process once
                simp.step();
                lastoptstep = std::chrono::steady_clock::now();
            }

            // Visualize at about 60 Hz
            std::chrono::steady_clock::duration sincerender = std::chrono::steady_clock::now() - lastrender;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(sincerender).count() > 17) { // 17 is about 60 Hz
                // Copy data out from NM_Simplex to update the triangle visualization
                for (unsigned int i = 0; i <= simp.n; ++i) {
                    tri_coords[i] = { simp.vertices[i][0], simp.vertices[i][1], 0.0 };
                    tri_values[i] = simp.values[i];
                }
                tfvp->reinit();
                v.poll();
                v.render();
                lastrender = std::chrono::steady_clock::now();
            }
        }
        std::vector<FLT> thebest = simp.best_vertex();
        std::cout << "Finished in " << simp.operation_count << " operations. Best approximation at: ("
                  << thebest[0] << "," << thebest[1] << ") has value " << simp.best_value() << std::endl;

        // Randomly set the next start position
        v1 = { rng.get(), rng.get() };
        v2 = { rng.get(), rng.get() };
        v3 = { rng.get(), rng.get() };
        i_vertices = { v1, v2, v3 };

        if (abs(thebest[0] - 1.0) < 1e-3 // Choose 1e-3 so that this will succeed with floats or doubles
            && abs(thebest[1] - 1.0) < 1e-3) {
            std::cout << "Test success" << std::endl;
            rtn = 0;
        } else {
            rtn = -1;
        }

        std::cout << "Pausing (Press Ctrl-v to re-run gradient descent)...\n";
        v.pauseOpen();
    }

    return rtn;
}
