/*
 * This example generates a number (n_points) of random (but bounded) coordinates and
 * uses the VoronoiVisual to display the coordinates as a map, with the order of
 * random-choice being used to colourize the Voronoi cells.
 *
 * This shows you how to use VoronoiVisual to visualize a surface from a non regular
 * grid (i.e. non mplot::Grid or mplot::HexGrid or mplot::HealpixVisual ordered values)
 *
 * It also shows how to automate the sceneview using mplot::direction_data objects.
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

    // Four scene views. Find your fave angle, and press Ctrl-e  to get a sceneview matrix array of your own!
    sm::mat<float, 4> sv1 = { 0.999439, 0.0101956, 0.031891, 0, 0.020491, 0.56702, -0.823449, 0, -0.0264784, 0.823641, 0.566493, 0, 0, 0, -16.0835, 1 };
    sm::mat<float, 4> sv2 = { 0.872552, -0.183199, 0.45287, 0, 0.34702, 0.88492, -0.310634, 0, -0.343846, 0.428199, 0.835713, 0, 0, 0, -16.0835, 1 };
    sm::mat<float, 4> sv3 = { 0.749983, -0.22668, -0.621404, 0, -0.660984, -0.221256, -0.71704, 0, 0.025049, 0.948505, -0.31577, 0, 0, 0, -16.0835, 1 };
    sm::mat<float, 4> sv4 = { -0.0563805, 0.974664, -0.216453, 0, -0.969398, -0.0015572, 0.245492, 0, 0.238935, 0.22367, 0.944924, 0, 0, 0, -16.0835, 1 };
    sm::vec<sm::mat<float, 4>, 4> svv = { sv1, sv2, sv3, sv4 };

    int fcount = 0;  // frame count
    int svcount = 0; // sceneview change count
    while (!v.readyToFinish()) {

        // Periodic changes every 300 frames
        if (fcount++% 100 == 0) {

            // Change the colour map
            vorvp->cm.setType (++cmap_t);
            vorvp->reinitColours();

            // Apply the sceneview from svv[svcount++ % 4] as a
            // direction_event::timed_transform. Create a direction_data object on the fly, setting
            // the time for the transform to be 25 frames
            mplot::direction_data dirn;
            dirn.event = mplot::direction_event::timed_transform;
            dirn.transform_time_frames = 25;
            dirn.sceneview = svv[svcount % 4];
            v.setCurrentDirectionEvent (dirn);

            ++svcount;
        }
        v.waitevents(0.018);
        v.render();
    }

    return rtn;
}
