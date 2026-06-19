/*
 * How to create your own mplot::Visual to either add additional keypress actions, or to
 * override the default actions.
 */
#include <iostream>
import mplot.visual;

// You can choose an OpenGL version to pass as template arg to mplot::Visual
constexpr int my_gl_version = mplot::gl::version_4_1;

struct myvisual final : public mplot::Visual<my_gl_version>
{
    // Boilerplate constructor (just copy this):
    myvisual (int width, int height, const std::string& title)
        : mplot::Visual<my_gl_version> (width, height, title) {}
    // Some attributes that you might need in your myvisual scene:
    bool moving = false;
protected:
    // Optionally, override key_callback() with a much sparser function:
    bool key_callback (int key, int scancode, int action, int mods) override
    {
        // Here, I've omitted all the normal keypress actions in Visual::key_callback,
        // except for one to close the program and one for help output:
        if (key == mplot::key::q && action == mplot::keyaction::press) {
            std::cout << "User requested exit.\n";
            this->state.set (mplot::visual_state::readyToFinish);
        }
        if (key == mplot::key::h && action == mplot::keyaction::press) {
            std::cout << "Help:\n";
            std::cout << "q: Exit program\n";
            std::cout << "h: This help\n";
        }
        // Then call the 'extra function', defined below
        this->key_callback_extra (key, scancode, action, mods);

        return false; // No need to re-render the window for either option
    }

    // Also optionally, add actions for extra keys:
    static constexpr bool debug_callback_extra = false;
    void key_callback_extra (int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) override
    {
        if constexpr (debug_callback_extra) {
            std::cout << "myvisual::key_callback_extra called for key=" << key << " scancode="
                      << scancode << " action=" << action << " and mods=" << mods << std::endl;
        }

        // As an example, bind the 'f' key to toggle the 'moving' attribute
        if (key == mplot::key::f && action == mplot::keyaction::press) {
            this->moving = this->moving ? false : true;
        }

        // You can require shift and a key like this:
        if (key == mplot::key::k && (mods & keymod::shift) && action == mplot::keyaction::press) {
            std::cout << "Shift-k was pressed\n";
        }

        // You can require NOT shift and a key like this:
        if (key == mplot::key::k && (mods & keymod::shift) == 0 && action == mplot::keyaction::press) {
            std::cout << "k was pressed without shift\n";
        }


        // Ctrl-key is:
        if (key == mplot::key::k && (mods & keymod::control) && action == mplot::keyaction::press) {
            std::cout << "Ctrl-k was pressed\n";
        }

        // combined:
        if (key == mplot::key::k
            && (mods & (keymod::shift | keymod::control)) == (keymod::shift | keymod::control)
            && action == mplot::keyaction::press) {
            std::cout << "Shift-Ctrl-k was pressed\n";
        }

        // Other keymod bits that you can use are alt and super:
        if (key == mplot::key::k && (mods & keymod::alt) && action == mplot::keyaction::press) {
            std::cout << "Alt-k was pressed\n";
        }
        if (key == mplot::key::k && (mods & keymod::super) && action == mplot::keyaction::press) {
            std::cout << "Super-k was pressed\n";
        }

        // In theory, mods can hold the status of caps_lock and num_lock. Doesn't work for Seb.
        if ((mods & keymod::caps_lock) && action == mplot::keyaction::press) {
            std::cout << "A key was pressed and Caps Lock was active\n";
        }
        if ((mods & keymod::num_lock) && action == mplot::keyaction::press) {
            std::cout << "A key was pressed and Num Lock was active\n";
        }

        // The caps lock key is also treated just as a key (works for Seb):
        if (key == mplot::key::caps_lock && action == mplot::keyaction::press) {
            std::cout << "Caps Lock was pressed\n";
        }

        // As well as press, you can test on release or repeat
        if (action == mplot::keyaction::release) {
            if (key == mplot::key::caps_lock) {
                std::cout << "Caps-lock released\n";
            } else {
                std::cout << "Key released\n";
            }
        }
        if (action == mplot::keyaction::repeat) {
            std::cout << "Key repeat\n";
        }

        // Add some additional help output:
        if (key == mplot::key::h && action == mplot::keyaction::press) {
            std::cout << "myvisual extra help:\n";
            std::cout << "f: 'move'\n";
            std::cout << "Shift-k: output a message to stdout\n";
            std::cout << "Ctrl-k: output a message to stdout\n";
            std::cout << "Any key release or repeat: output a message to stdout\n";
        }
    }
};

int main()
{
    myvisual v(600, 400, "Custom Visual: myvisual");
    v.addLabel ("Hello World! Try the key 'f' (then look at stdout). Try Shift-k or Ctrl-k", {0,0,0});
    while (!v.readyToFinish()) {
        v.waitevents (0.018);
        if (v.moving == true) {
            std::cout << "I moved...\n";
            v.moving = false;
        }
        v.render();
    }
    return 0;
}
