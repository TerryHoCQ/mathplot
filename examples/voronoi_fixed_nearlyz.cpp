/*
 * Make a very small Voronoi surface. Used to debug VoronoiVisual
 */
#include <memory>
#include <string>
#include <limits>
#include <vector>

import mplot.visual;
import mplot.voronoivisual;
import mplot.vectorvisual;

// Use key presses to change border_width, so extend mplot::Visual in the usual way
struct myvisual final : public mplot::Visual<>
{
    myvisual (int width, int height, const std::string& title) : mplot::Visual<> (width, height, title) {}
    float border_width = std::numeric_limits<float>::epsilon();
protected:
    void key_callback_extra (int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) override
    {
        if (key == mplot::key::up && action == mplot::keyaction::press) {
            this->border_width += 0.01f;
        }
        if (key == mplot::key::down && action == mplot::keyaction::press) {
            this->border_width -= 0.01f;
            if (this->border_width <= 0.0f) { this->border_width = std::numeric_limits<float>::epsilon(); }
        }
    }
};

int main()
{
    int rtn = -1;

    myvisual v(1024, 768, "VoronoiVisual");
    v.coordArrowsInScene (true);

    std::vector<sm::vec<float>> points = {
        {0,0,1},
        {1,0,1},
        {0,1,1},
        {1,1,1},
        {.5,.5,0.5},
    };
    std::vector<float> data = {1,2,3,4,5};

    sm::vec<float, 3> offset = { 0.0f };
    auto vorv = std::make_unique<mplot::VoronoiVisual<float>> (offset);
    vorv->set_parent (v.get_id());
    vorv->show_voronoi2d = true;
    vorv->debug_edges = true;
    vorv->debug_dataCoords = true;
    vorv->data_z_direction = sm::vec<float>{1,0,1};
    vorv->data_z_direction.renormalize();
    vorv->border_width  = v.border_width;
    vorv->setDataCoords (&points);
    vorv->setScalarData (&data);
    vorv->finalize();
    auto p_vorv = v.addVisualModel (vorv);

    offset[0] -= 0.5f;
    auto vvm = std::make_unique<mplot::VectorVisual<float, 3>>(offset);
    vvm->set_parent (v.get_id());
    vvm->thevec = p_vorv->data_z_direction;
    vvm->fixed_colour = true;
    vvm->thickness = 0.03f;
    vvm->single_colour = mplot::colour::dodgerblue2;
    vvm->addLabel ("Arrow gives data direction", {-0.8, -0.3, 0}, mplot::TextFeatures(0.1f));
    vvm->finalize();
    v.addVisualModel (vvm);

    while (!v.readyToFinish()) {
        if (p_vorv->border_width != v.border_width) {
            p_vorv->border_width = v.border_width;
            p_vorv->reinit();
        }
        v.render();
        v.waitevents (0.018);
    }

    return rtn;
}
