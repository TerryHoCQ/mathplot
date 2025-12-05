/*
 * Move one model (mplot::CoordArrows) around another (mplot::GeodesicVisual), as if it were
 * crawling over it. Demonstrates mplot::NavMesh, which allows you to move over a triangular
 * landscape model, following the exact contour defined by the landscape's mesh.
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <stdexcept>
#include <string>

#include <sm/vec>

#include <mplot/Visual.h>
#include <mplot/ColourMap.h>
#include <mplot/CoordArrows.h>
#include <mplot/ScatterVisual.h>
#include <mplot/GeodesicVisual.h>

int main (int argc, char** argv)
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "Crawling a surface with NavMesh features");
    v.rotateAboutNearest (true);

    // How big to make the sphere?
    constexpr float radius = 2.0f;
    // How many iterations for the geodesic? Try 2, 3 and 4
    int geo_itrns = 4;
    if (argc > 1) { geo_itrns = std::atoi (argv[1]); }
    // How high to hover the arrows
    constexpr float hoverheight = 0.05f;
    // Model locations within the scene
    //sm::vec<float, 3> arrows_loc = { 0.01f, radius + 1.5f * hoverheight, 0.2f };
    sm::vec<float, 3> arrows_loc = { 0.0f, radius + 1.5f * hoverheight, 0.0f };
    sm::vec<float, 3> sphere_loc = {};

    // A CoordArrows is our "crawling" agent
    auto ca = std::make_unique<mplot::CoordArrows<>> (arrows_loc);
    v.bindmodel (ca);
    ca->finalize();
    [[maybe_unused]] auto cap = v.addVisualModel (ca);

    // A ScatterVisual will show the agent's trail
    sm::vvec<sm::vec<float>> sv_points;
    sm::vvec<float> sv_data;
    auto sv = std::make_unique<mplot::ScatterVisual<float>> (sphere_loc);
    v.bindmodel (sv);
    sv->setDataCoords (&sv_points);
    sv->setScalarData (&sv_data);
    sv->radiusFixed = 0.015f;
    sv->cm.setType (mplot::ColourMapType::Plasma);
    sv->colourScale.compute_scaling (0.0f, 1.0f);
    sv->finalize();
    [[maybe_unused]] auto svp = v.addVisualModel (sv); // use svp->add (coord, value)

    // A sphere, approximated by an icosahedral geodesic, is our landscape
    mplot::ColourMap<float> cm (mplot::ColourMapType::Jet);
    auto cl = cm.convert (0.5f);
    auto gv = std::make_unique<mplot::GeodesicVisual<float>> (sphere_loc, radius);
    v.bindmodel (gv);
    gv->iterations = geo_itrns;
    std::string lbl = "GeodesicVisual with computed NavMesh";
    gv->addLabel (lbl, {0, -(radius + 0.1f), 0}, mplot::TextFeatures (0.06f));
    gv->cm.setType (mplot::ColourMapType::Jet);
    gv->colour_bb = cl;
    gv->finalize();
    auto gvp = v.addVisualModel (gv);
    // re-colour sphere with sequential colouring after construction
    gvp->data.linspace (0.0f, 1.0f, gvp->data.size());
    gvp->reinitColours();
    // Make the navmesh for the geodesic, this doesn't occur automatically and has to come after finalize()
    gvp->make_navmesh();

    // We're going to move the coordinate arrows forwards (along its z-axis), so that it 'orbits'
    float move_step = 0.1f; // 0.075 <= move_step and iterations 6 to fail
    sm::vec<float> mv_ca = sm::vec<float>::uz() * move_step;

    // The viewmatrices have to be passed to mplot::NavMesh::compute_mesh_movement
    sm::mat44<float> ca_view = cap->getViewMatrix();
    sm::mat44<float> sph_view = gvp->getViewMatrix();

    // Find the triangle that we're initially located above with
    // mplot::NavMesh::find_triangle_hit. This updates internal state in NavMesh. It could be
    // executed automatically in compute_mesh_movement
    auto[hp_scene, tn0, ti0] = gvp->navmesh->find_triangle_hit (ca_view, sph_view);
    std::cout << "Find hit finds hit point " << hp_scene << std::endl;

    int move_counter = 0;
    constexpr int move_max = 1000;
    while (!v.readyToFinish()) {

        // Wait .018 s and also poll for mouse/keyboard events
        v.waitevents (0.018);

        // Compute a new movement over the landscape mesh (the sphere)
        try {
            ca_view = gvp->navmesh->compute_mesh_movement (mv_ca, ca_view, sph_view, hoverheight);
        } catch (mplot::NavException& e) {
            if (e.m_type == mplot::NavException::type::off_edge) {
                std::cout << "You can handle movements that go off the edge of a flat model\n";
            }
            std::cout << "Exception navigating mesh at movement count " << move_counter << ": " << e.what() << std::endl;
            throw e;
        }

        // Update the viewmatrix of the coord arrows, setting its position within the scene
        cap->setViewMatrix (ca_view);

        // Compute the new location
        arrows_loc = (ca_view * sm::vec<float>{}).less_one_dim();

        // We're adding and rebuilding the not-very-optimized ScatterVisual, so if move_max is too
        // high, the program will slow down (too many tiny spheres!)
        if (move_counter++ < move_max) {
            svp->add (arrows_loc, static_cast<float>(move_counter) / move_max);
        }

        // Re-render the scene
        v.render();
    }

    v.keepOpen();

    return rtn;
}
