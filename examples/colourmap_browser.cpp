/*
 * Make a colourbar tester using a mplot::Grid/GridVisual
 *
 * This shows a min to max gradient of a ColourMap, with a decaying sine wave added to
 * the signal. Poor colour maps like Jet show structure in the features that is not part
 * of the data.
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <sm/vec>
#include <sm/grid>

#include <mplot/Visual.h>
#include <mplot/VisualDataModel.h>
#include <mplot/GridVisual.h>
#include <mplot/CyclicColourVisual.h>

struct myvisual final : public mplot::Visual<>
{
    myvisual (int width, int height, const std::string& title) : mplot::Visual<> (width, height, title) {}
    mplot::ColourMapType curr_map_type = mplot::ColourMapType::Plasma;
    bool forwards = true;
protected:
    void key_callback_extra (int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) override
    {
        if (key == mplot::key::right && (action == mplot::keyaction::press || action == mplot::keyaction::repeat)) {
            ++this->curr_map_type;
            this->forwards = true;
        }
        if (key == mplot::key::left && (action == mplot::keyaction::press || action == mplot::keyaction::repeat)) {
            --this->curr_map_type;
            this->forwards = false;
        }
        if (key == mplot::key::h && action == mplot::keyaction::press) { std::cout << "left/right switch maps\n"; }
    }
};

// Parameters for the fineness of the grid/cyclic wheels
static constexpr unsigned int Nside_w = 512;
static constexpr unsigned int Nside_h = 256;

mplot::VisualModel<>* addmap (myvisual& v, mplot::ColourMapType display_map_type, const sm::grid<>& grid, const std::vector<float>& data)
{
    mplot::VisualModel<>* vmp = nullptr;
    mplot::ColourMap<float> nextmap (display_map_type);
    if (nextmap.flags.test(mplot::ColourMapFlags::cyclic) == true) {
        sm::vec<float, 3> offset = {0,0,0};
        auto cv = std::make_unique<mplot::CyclicColourVisual<float>>(offset);
        v.bindmodel (cv);
        cv->outer_radius = 0.6;
        cv->inner_radius = 0.2;
        cv->numsegs = Nside_w;
        cv->numrings = Nside_h;
        cv->cm = nextmap;
        cv->draw_ticks = false;
        cv->addLabel (cv->cm.getTypeStr() + std::string(" (") + cv->cm.getFlagsStr() + std::string(")"),
                      sm::vec<float>({-1.3, -0.4, 0}), mplot::TextFeatures(0.05f));
        cv->finalize();
        vmp = v.addVisualModel (cv);
    } else {
        sm::vec<float, 3> offset = { -0.5f * grid.width(), -0.5f * grid.height(), 0.0f };
        auto gv = std::make_unique<mplot::GridVisual<float>>(&grid, offset);
        v.bindmodel (gv);
        gv->gridVisMode = mplot::GridVisMode::Triangles;
        gv->twodimensional (true);
        gv->setScalarData (&data);
        gv->cm = nextmap;
        gv->zScale.null_scaling();
        gv->addLabel (gv->cm.getTypeStr() + std::string(" (") + gv->cm.getFlagsStr() + std::string(")"),
                      sm::vec<float>({0,-0.1,0}), mplot::TextFeatures(0.05f));
        gv->finalize();
        vmp = v.addVisualModel (gv);
    }
    return vmp;
}

int main()
{
    myvisual v(2100, 1100, "Colourbar perceptual uniformity test");
    v.setSceneTrans (sm::vec<float,3>{ float{-0.00636619}, float{0.0518834}, float{-3} });

    // Create a grid for the colourmaps
    constexpr float barw = 2.56f;
    constexpr float barh = 0.5f;
    constexpr sm::vec<float, 2> grid_spacing = {barw/static_cast<float>(Nside_w), barh/static_cast<float>(Nside_h)};
    sm::grid grid(Nside_w, Nside_h, grid_spacing);

    // Our data is a ramp with a sine wave embossed on it
    std::vector<float> data(grid.n(), 0.0);
    for (unsigned int ri=0; ri<grid.n(); ++ri) {
        auto x = grid[ri][0];
        auto y = grid[ri][1];
        data[ri] = x / grid.width() + 0.1f * (y / grid.height()) * (y / grid.height()) * std::sin (120.0f * x);
    }

    mplot::ColourMapType display_map_type = v.curr_map_type;
    mplot::VisualModel<>* gvp = addmap (v, v.curr_map_type, grid, data);

    while (v.readyToFinish() == false) {
        v.render();
        v.waitevents (0.017);
        if (v.curr_map_type != display_map_type) {
            // Change to v.curr_map_type
            mplot::ColourMap<float> nextmap(v.curr_map_type);
            if (nextmap.flags.test (mplot::ColourMapFlags::one_d) == true) {
                // Update the map
                v.removeVisualModel (gvp);
                gvp = addmap (v, v.curr_map_type, grid, data);
                display_map_type = v.curr_map_type;
            } else {
                // The map wasn't 1D, so skip
                if (v.forwards) { ++v.curr_map_type; } else { --v.curr_map_type; }
            }
        }
    }

    return 0;
}
