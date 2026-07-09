/*
 * This program was a test prog to help me decide how to make
 * VisualModel::computeIcosahedron. It draws a scatter plot of icosahedron vertices,
 * triangles for the faces and another set of scatter plot spheres for the face centres.
 */

#include <memory>
#include <iostream>
#include <cmath>
#include <array>

import sm.geometry;
import sm.geometry_polyhedra;
import mplot.visual;
import mplot.scattervisual;
import mplot.trianglevisual;

int main()
{
    int rtn = -1;

    mplot::Visual v(1024, 768, "mplot::ScatterVisual Icosahedron");
    v.showCoordArrows (true);
    v.lightingEffects();

    sm::geometry::polyhedron<float> ico = sm::geometry::icosahedron<float>();

    // Coordinates of face centres
    sm::vvec<sm::vec<float, 3>> fcentres(20, {0.0, 0.0, 0.0});
    for (unsigned int i = 0; i < 20; ++i) {
        fcentres[i] = (ico.vertices[ico.faces[i][0]] + ico.vertices[ico.faces[i][1]] + ico.vertices[ico.faces[i][2]]).as_float() / 3.0f;
    }
    try {
        sm::vec<float, 3> offset = { 0.0, 0.0, 0.0 };
        sm::scale<float, float> scale;
        scale.set_params (1.0, 0.0);

        sm::vvec<float> data(12, 0.06f);
        sm::vvec<float> data2(20, 0.95f);

        auto sv = std::make_unique<mplot::ScatterVisual<float>> (offset);
        sv->set_parent (v.get_id());
        sv->setDataCoords (&ico.vertices);
        sv->setScalarData (&data);
        sv->radiusFixed = 0.01f;
        sv->colourScale = scale;
        sv->cm.setType (mplot::ColourMapType::Plasma);
        sv->labelIndices = true;
        sv->finalize();
        v.addVisualModel (sv);

        // Use a second scatter visual to show the centre of each face, numbered in a different colour
        sv = std::make_unique<mplot::ScatterVisual<float>> (offset);
        sv->set_parent (v.get_id());
        sv->setDataCoords (&fcentres);
        sv->setScalarData (&data2);
        sv->radiusFixed = 0.01f;
        sv->colourScale = scale;
        sv->cm.setType (mplot::ColourMapType::Plasma);
        sv->labelIndices = true;
        sv->finalize();
        v.addVisualModel (sv);

        // Triangle visuals for the faces
        mplot::ColourMap<float> cm(mplot::ColourMapType::Jet);
        for (unsigned int i = 0; i < 20; ++i) {
            std::array<float, 3> colr = cm.convert (i/20.0f);
            auto tv = std::make_unique<mplot::TriangleVisual<>> (offset,
                                                                 ico.vertices[ico.faces[i][0]],
                                                                 ico.vertices[ico.faces[i][1]],
                                                                 ico.vertices[ico.faces[i][2]],
                                                                 colr);
            tv->set_parent (v.get_id());
            tv->setAlpha (0.8f);
            tv->finalize();
            v.addVisualModel (tv);
        }

        v.keepOpen();

    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        rtn = -1;
    }

    return rtn;
}
