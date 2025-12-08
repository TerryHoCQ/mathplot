#pragma once

#include <mplot/keys.h>
#include <QKeyEvent>

namespace mplot::qt
{
    // Pass in a Qt key code and receive back the mplot (i.e. GLFW) key code.
    constexpr int qtkey_to_mplotkey (int qt_keycode)
    {
        int mkc = mplot::key::unknown;
        switch (qt_keycode) {

        case Qt::Key_Space:
            mkc = mplot::key::space;
            break;
        case Qt::Key_Apostrophe:
            mkc = mplot::key::apostrophe;
            break;
        case Qt::Key_Comma:
            mkc = mplot::key::comma;
            break;
        case Qt::Key_Minus:
            mkc = mplot::key::minus;
            break;
        case Qt::Key_Period:
            mkc = mplot::key::period;
            break;
        case Qt::Key_Slash:
            mkc = mplot::key::slash;
            break;

        case Qt::Key_0:
            mkc = mplot::key::n0;
            break;
        case Qt::Key_1:
            mkc = mplot::key::n1;
            break;
        case Qt::Key_2:
            mkc = mplot::key::n2;
            break;
        case Qt::Key_3:
            mkc = mplot::key::n3;
            break;
        case Qt::Key_4:
            mkc = mplot::key::n4;
            break;
        case Qt::Key_5:
            mkc = mplot::key::n5;
            break;
        case Qt::Key_6:
            mkc = mplot::key::n6;
            break;
        case Qt::Key_7:
            mkc = mplot::key::n7;
            break;
        case Qt::Key_8:
            mkc = mplot::key::n8;
            break;
        case Qt::Key_9:
            mkc = mplot::key::n9;
            break;

        case Qt::Key_Semicolon:
            mkc = mplot::key::semicolon;
            break;
        case Qt::Key_Equal:
            mkc = mplot::key::equal;
            break;

        case Qt::Key_A:
            mkc = mplot::key::a;
            break;
        case Qt::Key_B:
            mkc = mplot::key::b;
            break;
        case Qt::Key_C:
            mkc = mplot::key::c;
            break;
        case Qt::Key_D:
            mkc = mplot::key::d;
            break;
        case Qt::Key_E:
            mkc = mplot::key::e;
            break;
        case Qt::Key_F:
            mkc = mplot::key::f;
            break;
        case Qt::Key_G:
            mkc = mplot::key::g;
            break;
        case Qt::Key_H:
            mkc = mplot::key::h;
            break;
        case Qt::Key_I:
            mkc = mplot::key::i;
            break;
        case Qt::Key_J:
            mkc = mplot::key::j;
            break;
        case Qt::Key_K:
            mkc = mplot::key::k;
            break;
        case Qt::Key_L:
            mkc = mplot::key::l;
            break;
        case Qt::Key_M:
            mkc = mplot::key::m;
            break;
        case Qt::Key_N:
            mkc = mplot::key::n;
            break;
        case Qt::Key_O:
            mkc = mplot::key::o;
            break;
        case Qt::Key_P:
            mkc = mplot::key::p;
            break;
        case Qt::Key_Q:
            mkc = mplot::key::q;
            break;
        case Qt::Key_R:
            mkc = mplot::key::r;
            break;
        case Qt::Key_S:
            mkc = mplot::key::s;
            break;
        case Qt::Key_T:
            mkc = mplot::key::t;
            break;
        case Qt::Key_U:
            mkc = mplot::key::u;
            break;
        case Qt::Key_V:
            mkc = mplot::key::v;
            break;
        case Qt::Key_W:
            mkc = mplot::key::w;
            break;
        case Qt::Key_X:
            mkc = mplot::key::x;
            break;
        case Qt::Key_Y:
            mkc = mplot::key::y;
            break;
        case Qt::Key_Z:
            mkc = mplot::key::z;
            break;

        case Qt::Key_BracketLeft:
            mkc = mplot::key::left_bracket;
            break;
        case Qt::Key_Backslash:
            mkc = mplot::key::backslash;
            break;
        case Qt::Key_BracketRight:
            mkc = mplot::key::right_bracket;
            break;
        case Qt::Key_QuoteLeft:
            mkc = mplot::key::grave_accent;
            break;
        case Qt::Key_exclamdown: // guess
            mkc = mplot::key::world_1;
            break;
        case Qt::Key_cent:       // guess
            mkc = mplot::key::world_2;
            break;

        case Qt::Key_Escape:
            mkc = mplot::key::escape;
            break;
        case Qt::Key_Enter:
            mkc = mplot::key::enter;
            break;
        case Qt::Key_Tab:
            mkc = mplot::key::tab;
            break;
        case Qt::Key_Backspace:
            mkc = mplot::key::backspace;
            break;
        case Qt::Key_Insert:
            mkc = mplot::key::insert;
            break;
        case Qt::Key_Delete:
            mkc = mplot::key::delete_key;
            break;
        case Qt::Key_Right:
            mkc = mplot::key::right;
            break;
        case Qt::Key_Left:
            mkc = mplot::key::left;
            break;
        case Qt::Key_Down:
            mkc = mplot::key::down;
            break;
        case Qt::Key_Up:
            mkc = mplot::key::up;
            break;
        case Qt::Key_PageUp:
            mkc = mplot::key::page_up;
            break;
        case Qt::Key_PageDown:
            mkc = mplot::key::page_down;
            break;
        case Qt::Key_Home:
            mkc = mplot::key::home;
            break;
        case Qt::Key_End:
            mkc = mplot::key::end;
            break;
        case Qt::Key_CapsLock:
            mkc = mplot::key::caps_lock;
            break;
        case Qt::Key_ScrollLock:
            mkc = mplot::key::scroll_lock;
            break;
        case Qt::Key_NumLock:
            mkc = mplot::key::num_lock;
            break;
        case Qt::Key_Print:
            mkc = mplot::key::print_screen;
            break;
        case Qt::Key_Pause:
            mkc = mplot::key::pause;
            break;

        case Qt::Key_F1:
            mkc = mplot::key::f1;
            break;
        case Qt::Key_F2:
            mkc = mplot::key::f2;
            break;
        case Qt::Key_F3:
            mkc = mplot::key::f3;
            break;
        case Qt::Key_F4:
            mkc = mplot::key::f4;
            break;
        case Qt::Key_F5:
            mkc = mplot::key::f5;
            break;
        case Qt::Key_F6:
            mkc = mplot::key::f6;
            break;
        case Qt::Key_F7:
            mkc = mplot::key::f7;
            break;
        case Qt::Key_F8:
            mkc = mplot::key::f8;
            break;
        case Qt::Key_F9:
            mkc = mplot::key::f9;
            break;
        case Qt::Key_F10:
            mkc = mplot::key::f10;
            break;
        case Qt::Key_F11:
            mkc = mplot::key::f11;
            break;
        case Qt::Key_F12:
            mkc = mplot::key::f12;
            break;
        case Qt::Key_F13:
            mkc = mplot::key::f13;
            break;
        case Qt::Key_F14:
            mkc = mplot::key::f14;
            break;
        case Qt::Key_F15:
            mkc = mplot::key::f15;
            break;
        case Qt::Key_F16:
            mkc = mplot::key::f16;
            break;
        case Qt::Key_F17:
            mkc = mplot::key::f17;
            break;
        case Qt::Key_F18:
            mkc = mplot::key::f18;
            break;
        case Qt::Key_F19:
            mkc = mplot::key::f19;
            break;
        case Qt::Key_F20:
            mkc = mplot::key::f20;
            break;
        case Qt::Key_F21:
            mkc = mplot::key::f21;
            break;
        case Qt::Key_F22:
            mkc = mplot::key::f22;
            break;
        case Qt::Key_F23:
            mkc = mplot::key::f23;
            break;
        case Qt::Key_F24:
            mkc = mplot::key::f24;
            break;
        case Qt::Key_F25:
            mkc = mplot::key::f25;
            break;

        case Qt::Key_division:
            mkc = mplot::key::kp_divide;
            break;
        case Qt::Key_multiply:
            mkc = mplot::key::kp_multiply;
            break;

        case Qt::Key_Shift:
            mkc = mplot::key::left_shift;
            break;
        case Qt::Key_Control:
            mkc = mplot::key::left_control;
            break;
        case Qt::Key_Alt:
            mkc = mplot::key::left_alt;
            break;
        case Qt::Key_AltGr:
            mkc = mplot::key::right_alt;
            break;
        case Qt::Key_Super_L:
            mkc = mplot::key::left_super;
            break;
        case Qt::Key_Super_R:
            mkc = mplot::key::right_super;
            break;

        default:
            break;
        }
        return mkc;
    }
}
