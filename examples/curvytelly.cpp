/*
 * Demonstrate the CurvyTellyVisual by showing an image
 */
#include <memory>

import sm.mathconst;
import sm.vec;
import sm.vvec;
import sm.grid;
import sm.quaternion;
import mplot.loadpng;
import mplot.colourmap;
import mplot.visual;
import mplot.curvytellyvisual;

int main()
{
    mplot::Visual<> v(1600, 1000, "CurvyTellyVisual showing an image");

    // Load an image (you have to run the program from ./build/
    sm::vvec<float> image_data;
    sm::vec<unsigned int, 2> dims = mplot::loadpng ("../examples/horsehead_reduced.png", image_data);

    // CurvyTellyVisual needs a Grid as an underlying data structure
    sm::vec<float, 2> grid_spacing = { 0.1f, 0.01f };
    sm::grid grid(dims[0], dims[1], grid_spacing);

    sm::vec<float> offset = { 0, 0, 0 };
    auto ctv = std::make_unique<mplot::CurvyTellyVisual<float>>(&grid, offset);
    ctv->set_parent (v.get_id());
    ctv->setScalarData (&image_data);
    ctv->cm.setType (mplot::ColourMapType::Magma);
    ctv->radius = 10.0f;     // The radius of curvature of the telly
    ctv->centroidize = true; // Ensures the centre of the VisualModel is the 'middle of the screen' (it's centroid)
    ctv->angle_to_subtend = sm::mathconst<float>::pi_over_3; // 2pi is default
    ctv->frame_width = 0.1f; // Show a frame around the image
    ctv->frame_clr = mplot::colour::navy;
    ctv->finalize();
    v.addVisualModel (ctv);

    // To make this view in the correct orientation as if it were a TV, we have to rotate & translate the scene.
    v.setSceneTrans (sm::vec<float,3>{ float{0}, float{0}, float{-14} });
    v.setSceneRotation (sm::quaternion<float>{ float{-0.5}, float{0.5}, float{-0.5}, float{-0.5} });

    v.keepOpen();

    return 0;
}
