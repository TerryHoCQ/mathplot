/*
 * Illustrate use of unsaturating 1D map
 */

#include <iostream>
#include <vector>
#include <cmath>

#include <sm/mathconst>
#include <sm/vec>

#include <mplot/Visual.h>
#include <mplot/ColourMap.h>
#include <mplot/HSVWheelVisual.h>

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

// In this example, I'll create a special visual to show the colours
struct SquareGridVisual : public mplot::VisualModel<>
{
    SquareGridVisual(const sm::vec<float> _offset, mplot::ColourMapType mymap) : mplot::VisualModel<> (_offset)
    {
        this->colourMap.setType (mymap);
        // We're going to 'act 2D'
        this->colourMap.set_act_2d (true);
    }

    // A colourMap member attribute
    mplot::ColourMap<float> colourMap;

    // initializeVertices is the standard function that we override when extending
    // VisualModel. Here, we're going to draw a grid of squares, with colour chosen
    // using their x and y coordinates
    void initializeVertices() override
    {
        // Some compile-time values used in the loop. The first says draw a square polygon with 4 sides. Change to 5 and see what happens.
        static constexpr int square_has_four_segments = 4;
        // Whats the distance from the centre of one square in the grid to the next?
        static constexpr float element_to_element_distance = 0.2f;
        // In a 'flat polygon', The radius is defined as the distance to a vertex, so,
        // for example, to get a square of width 2, set the radius to sqrt(2). Here, I use 0.97f to ensure a thin gap between squares
        static constexpr float square_centre_to_vertex = 0.97f * (element_to_element_distance / 2.0f) * sm::mathconst<float>::root_2;
        // Polygons have a vertex pointing 'up' by default, so we have to rotate
        static constexpr float square_needs_rotation = sm::mathconst<float>::pi_over_4;
        // How many squares along a side of the grid?
        static constexpr int num_elements_on_side = 60;
        // That number - 1:
        static constexpr float num_elements_less_one = num_elements_on_side - 1.0f;

        // Loop through, creating a grid of squares:
        for (int x = 0; x < num_elements_on_side; x++) {
            for (int y = 0; y < num_elements_on_side; y++) {

                // Create grid element position from x and y.
                sm::vec<float> element_pos = { static_cast<float>(x), static_cast<float>(y), 0.0f };
                element_pos *= element_to_element_distance;

                // We call the 2 argument overload of ColourMap::convert, making sure
                // that the range of each argument is [0,1] (hence the
                // divisions). Inside ColourMap::convert, the two numbers are treated as
                // coordinates and turned into an angle about (0.5,0.5) and a
                // radius. The angle becomes the hue and the radius the saturation. The
                // 'value' for the hsv colour is 1 by default. This can be modified by
                // calling ColourMap::setVal(). The starting colour for the hue is red,
                // which can also be modified, using setHue or setHueRotation.
                std::array<float, 3> element_colour = this->colourMap.convert (x/num_elements_less_one, y/num_elements_less_one);

                // We use a 'flat poly' primitive to draw a square. ux and uy are unit vectors that control the orientation of the polygon
                this->computeFlatPoly (element_pos, sm::vec<>::ux(), sm::vec<>::uy(), element_colour,
                                       square_centre_to_vertex, square_has_four_segments, square_needs_rotation);
            }
        }
    }
};

int main()
{
    // The main function is simple. Create a mplot::Visual, add a single SquareGridVisual and then 'keep it open'
    std::string titlestr = "1D colour maps with 2D inputs (desaturating)";
    myvisual v(1600, 1000, titlestr);
    v.backgroundBlack();
    v.setSceneTrans (-5.60868263,-5.17123413,-29.2000771); // numbers obtained by pressing 'z' and seeing stdout

    // This is addmap
    auto hsv_vis = std::make_unique<SquareGridVisual>(sm::vec<float>{ 0.0f, 0.0f, 0.0f }, v.curr_map_type);
    v.bindmodel (hsv_vis);
    hsv_vis->addLabel (hsv_vis->colourMap.getTypeStr() + std::string(" (") + hsv_vis->colourMap.getFlagsStr() + std::string(")"),
                       sm::vec<float>({0,-1,0}), mplot::TextFeatures(0.24f, mplot::colour::white));
    hsv_vis->finalize();
    auto gvp = v.addVisualModel (hsv_vis);

    mplot::ColourMapType display_map_type = v.curr_map_type;

    while (v.readyToFinish() == false) {
        v.render();
        v.waitevents (0.017);
        if (v.curr_map_type != display_map_type) {
            // Change to v.curr_map_type
            mplot::ColourMap<float> nextmap(v.curr_map_type);
            if (nextmap.flags.test (mplot::ColourMapFlags::one_d) == true) {
                // Update the map
                v.removeVisualModel (gvp);

                auto hsv_vis = std::make_unique<SquareGridVisual>(sm::vec<float>{ 0.0f, 0.0f, 0.0f }, v.curr_map_type);
                v.bindmodel (hsv_vis);
                hsv_vis->addLabel (hsv_vis->colourMap.getTypeStr() + std::string(" (") + hsv_vis->colourMap.getFlagsStr() + std::string(")"),
                                   sm::vec<float>({0,-1,0}), mplot::TextFeatures(0.24f, mplot::colour::white));
                hsv_vis->finalize();
                gvp = v.addVisualModel (hsv_vis);

                display_map_type = v.curr_map_type;
            } else {
                // The map wasn't 1D, so skip
                if (v.forwards) { ++v.curr_map_type; } else { --v.curr_map_type; }
            }
        }
    }

    return 0;
}
