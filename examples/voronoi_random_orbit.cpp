/*
 * This example generates a number (n_points) of random (but bounded) coordinates and
 * uses the VoronoiVisual to display the coordinates as a map, with the order of
 * random-choice being used to colourize the Voronoi cells.
 *
 * This shows you how to use VoronoiVisual to visualize a surface from a non regular
 * grid (i.e. non mplot::Grid or mplot::HexGrid or mplot::HealpixVisual ordered values)
 *
 * It also shows how to automate the sceneview using mplot::direction_data objects, in this case it
 * does the orbit movement.
 *
 * Author Seb James
 * Date 2024
 * Direction updates 2026
 */
#include <iostream>
#include <memory>
#include <vector>
#include <cmath>

import sm.vec;
import sm.random;
import sm.mat;
import mplot.colourmap;
import mplot.visual;
import mplot.voronoivisual;

static constexpr int n_points = 1000;

int main()
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "VoronoiVisual");

    sm::rand_uniform<float> rngxy(-2.0f, 2.0f, 1000);
    sm::rand_uniform<float> rngz(0.8f, 1.0f, 1000);

    // make n_points random coordinates
    std::vector<sm::vec<float>> points(n_points);
    std::vector<float> data(n_points);

    for (unsigned int i = 0; i < n_points; ++i) {
        points[i] = { rngxy.get(), rngxy.get(), rngz.get() };
        data[i] = static_cast<float>(i) / n_points;
    }

    mplot::ColourMapType cmap_t = mplot::ColourMapType::Plasma;

    sm::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<mplot::VoronoiVisual<float>> (offset);
    vorv->set_parent (v.get_id());
    vorv->show_voronoi2d = true; // true to show the 2D voronoi edges
    vorv->debug_dataCoords = false; // true to show coordinate spheres
    float length_scale = 4.0f / std::sqrt (n_points);
    vorv->border_width  = length_scale;
    vorv->cm.setType (cmap_t);
    vorv->setDataCoords (&points);
    vorv->setScalarData (&data);
    vorv->finalize();
    auto vorvp = v.addVisualModel (vorv);

    // I'm going to rotate the Voronoi surface so that it's like a landscape. Rotate -90 degrees about x axis
    sm::mat<float, 4> rot (sm::quaternion<float>(sm::vec<float>::ux(), -sm::mathconst<float>::pi_over_2));
    vorvp->setViewMatrix (rot);

    // Initial scene view. Find your fave angle, and press Ctrl-e  to get a sceneview matrix array of your own!

    int fcount = 0;  // frame count
    while (!v.readyToFinish()) {

        // Periodic changes every 300 frames
        if (fcount++% 2000 == 0) {

            // Change the colour map
            vorvp->cm.setType (++cmap_t);
            vorvp->reinitColours();

            // Apply the sceneview from svv[svcount++ % 4] as a
            // direction_event::timed_transform. Create a direction_data object on the fly, setting
            // the time for the transform to be 25 frames
            mplot::direction_data dirn;
            dirn.event = mplot::direction_event::timed_orbit;
            dirn.transform_time_frames = 1500;
            dirn.min_jerk = true;
            dirn.orbit_centre = sm::vec<>{0,0,0};
            dirn.orbit_axis = v.scene_up;
            dirn.orbit_angle = sm::mathconst<float>::two_pi;
            v.setCurrentDirectionEvent (dirn);
        }
        v.waitevents(0.018);
        v.render();
    }

    return rtn;
}
