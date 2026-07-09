/*
 * Visualize a sequence of icosahedral geodesics
 */

#include <memory>
#include <string>
#include <format>

import mplot.visual;
import mplot.geodesicvisual;

int main()
{
    mplot::Visual v(1024, 768, "Geodesic Polyhedra (ordered vertices/faces)");
    v.showCoordArrows (true);
    // Set the Visual to rotate about the nearest VisualModel (Change at runtime with Ctrl-k)
    v.rotateAboutNearest (true);
    // In this example, use the 'rotate about a scene vertical axis' mode
    v.rotateAboutVertical (true);

    sm::vec<float, 3> offset = {};
    sm::vec<float, 3> step = { 2.2f };

    mplot::ColourMap<float> cm (mplot::ColourMapType::Jet);
    int imax = 4;
    for (int i = 0; i < imax; ++i) {
        auto cl = cm.convert (i / static_cast<float>(imax - 1));
        auto gv1 = std::make_unique<mplot::GeodesicVisual<float>> (offset + step * i, 0.9f);
        gv1->set_parent (v.get_id());
        gv1->iterations = i;
        gv1->cm.setType (mplot::ColourMapType::Jet);
        gv1->colour_bb = cl;
        gv1->finalize();

        // re-colour after construction
        auto gv1p = v.addVisualModel (gv1);
        gv1p->addLabel (std::format ("iteration = {} ({} faces, {} vertices)", i, gv1p->n_faces, gv1p->n_geo_verts),
                        {0, -1, 0}, mplot::TextFeatures(0.06f));
        float imax_mult = 1.0f / static_cast<float>(imax);
        // sequential colouring:
        size_t sz1 = gv1p->data.size();
        gv1p->data.linspace (0.0f, 1+i * imax_mult, sz1);
        gv1p->reinitColours();
    }

    v.keepOpen();
}
