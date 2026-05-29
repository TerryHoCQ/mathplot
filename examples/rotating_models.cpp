/*
 * Visualize a single vector
 *
 * Do the rotations without rebuilding the OpenGL model each time.
 */
#include <memory>
#include <string>

import sm.vec;
import sm.quaternion;

import mplot.visual;
import mplot.colourmap;
import mplot.vectorvisual;
import mplot.geodesicvisual;

int main()
{
    mplot::Visual v(1024, 768, "mplot::VectorVisual");
    v.lightingEffects();
    v.showCoordArrows (true);
    v.coordArrowsInScene (true);

    sm::vec<float> offset = {1,0,0};


    auto vvm = std::make_unique<mplot::VectorVisual<float, 3>>(offset);
    vvm->set_parent (v.get_id());
    vvm->thevec = {1,1,1};
    vvm->fixed_colour = true;
    vvm->single_colour = mplot::colour::crimson;
    vvm->addLabel ("VisualModel rotation (text fixed)", {-0.8, -0.5, 0}, mplot::TextFeatures(0.1f));
    vvm->finalize();
    auto ptr = v.addVisualModel (vvm);

    auto gv1 = std::make_unique<mplot::GeodesicVisual<float>> (-offset, 0.9f);
    gv1->set_parent (v.get_id());
    gv1->iterations = 0;
    std::string lbl = std::string("Icosahedron");
    gv1->addLabel (lbl, {0, -1, 0}, mplot::TextFeatures(0.1f));
    gv1->cm.setType (mplot::ColourMapType::Jet);
    gv1->finalize();
    // re-colour after construction
    auto ptr2 = v.addVisualModel (gv1);
    // sequential colouring:
    size_t sz1 = ptr2->data.size();
    ptr2->data.linspace (0.0f, 1.0f, sz1);
    ptr2->reinitColours();

    float angle_per_frame = 0.05f;
    sm::vec<float> axis = {0,1,0};

    // Set up a rotation about the z axis
    sm::quaternion<float> qr (axis, angle_per_frame);

    // A rotation for the icosahedron
    axis = {1, 0, 0};
    sm::quaternion<float> qr_icos (axis, angle_per_frame / 2.0f);

    while (!v.readyToFinish()) {
        v.render();
        v.wait (0.01);

        // Add an additional rotation of the view of the VisualModel (including a rotation of texts):
        ptr->addViewRotationFixTexts (qr);

        // Add an additional rotation of the view of the VisualModel (but texts are fixed):
        ptr2->addViewRotationFixTexts (qr_icos);
    }

    return 0;
}
