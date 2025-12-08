#pragma once

#include <mplot/keys.h>
#include <wx/event.h>

// WXK_ENTER WXK_APOSTROPHE and WXK_META have been omitted

namespace mplot::wx
{
    // Pass in a wxWidgets key code and receive back the mplot (i.e. GLFW) key code.
    constexpr int wxkey_to_mplotkey(int wx_keycode)
    {
        int mkc = mplot::key::unknown;
        switch (wx_keycode) {

        case WXK_SPACE:
            mkc = mplot::key::space;
            break;
        case ',':
            mkc = mplot::key::comma;
            break;
        case '-':
            mkc = mplot::key::minus;
            break;
        case '.':
            mkc = mplot::key::period;
            break;
        case '/':
            mkc = mplot::key::slash;
            break;

        case '0':
            mkc = mplot::key::n0;
            break;
        case '1':
            mkc = mplot::key::n1;
            break;
        case '2':
            mkc = mplot::key::n2;
            break;
        case '3':
            mkc = mplot::key::n3;
            break;
        case '4':
            mkc = mplot::key::n4;
            break;
        case '5':
            mkc = mplot::key::n5;
            break;
        case '6':
            mkc = mplot::key::n6;
            break;
        case '7':
            mkc = mplot::key::n7;
            break;
        case '8':
            mkc = mplot::key::n8;
            break;
        case '9':
            mkc = mplot::key::n9;
            break;

        case ';':
            mkc = mplot::key::semicolon;
            break;
        case '=':
            mkc = mplot::key::equal;
            break;

        case 'A':
            mkc = mplot::key::a;
            break;
        case 'B':
            mkc = mplot::key::b;
            break;
        case 'C':
            mkc = mplot::key::c;
            break;
        case 'D':
            mkc = mplot::key::d;
            break;
        case 'E':
            mkc = mplot::key::e;
            break;
        case 'F':
            mkc = mplot::key::f;
            break;
        case 'G':
            mkc = mplot::key::g;
            break;
        case 'H':
            mkc = mplot::key::h;
            break;
        case 'I':
            mkc = mplot::key::i;
            break;
        case 'J':
            mkc = mplot::key::j;
            break;
        case 'K':
            mkc = mplot::key::k;
            break;
        case 'L':
            mkc = mplot::key::l;
            break;
        case 'M':
            mkc = mplot::key::m;
            break;
        case 'N':
            mkc = mplot::key::n;
            break;
        case 'O':
            mkc = mplot::key::o;
            break;
        case 'P':
            mkc = mplot::key::p;
            break;
        case 'Q':
            mkc = mplot::key::q;
            break;
        case 'R':
            mkc = mplot::key::r;
            break;
        case 'S':
            mkc = mplot::key::s;
            break;
        case 'T':
            mkc = mplot::key::t;
            break;
        case 'U':
            mkc = mplot::key::u;
            break;
        case 'V':
            mkc = mplot::key::v;
            break;
        case 'W':
            mkc = mplot::key::w;
            break;
        case 'X':
            mkc = mplot::key::x;
            break;
        case 'Y':
            mkc = mplot::key::y;
            break;
        case 'Z':
            mkc = mplot::key::z;
            break;

        case '[':
            mkc = mplot::key::left_bracket;
            break;
        case '\\':
            mkc = mplot::key::backslash;
            break;
        case ']':
            mkc = mplot::key::right_bracket;
            break;
        case '`':
            mkc = mplot::key::grave_accent;
            break;

        case WXK_ESCAPE:
            mkc = mplot::key::escape;
            break;
        case WXK_TAB:
            mkc = mplot::key::tab;
            break;
        case WXK_BACK:
            mkc = mplot::key::backspace;
            break;
        case WXK_INSERT:
            mkc = mplot::key::insert;
            break;
        case WXK_DELETE:
            mkc = mplot::key::delete_key;
            break;

        case WXK_RIGHT:
            mkc = mplot::key::right;
            break;
        case WXK_LEFT:
            mkc = mplot::key::left;
            break;
        case WXK_DOWN:
            mkc = mplot::key::down;
            break;
        case WXK_UP:
            mkc = mplot::key::up;
            break;

        case WXK_PAGEUP:
            mkc = mplot::key::page_up;
            break;
        case WXK_PAGEDOWN:
            mkc = mplot::key::page_down;
            break;
        case WXK_HOME:
            mkc = mplot::key::home;
            break;
        case WXK_END:
            mkc = mplot::key::end;
            break;

        case WXK_F1:
            mkc = mplot::key::f1;
            break;
        case WXK_F2:
            mkc = mplot::key::f2;
            break;
        case WXK_F3:
            mkc = mplot::key::f3;
            break;
        case WXK_F4:
            mkc = mplot::key::f4;
            break;
        case WXK_F5:
            mkc = mplot::key::f5;
            break;
        case WXK_F6:
            mkc = mplot::key::f6;
            break;
        case WXK_F7:
            mkc = mplot::key::f7;
            break;
        case WXK_F8:
            mkc = mplot::key::f8;
            break;
        case WXK_F9:
            mkc = mplot::key::f9;
            break;
        case WXK_F10:
            mkc = mplot::key::f10;
            break;
        case WXK_F11:
            mkc = mplot::key::f11;
            break;
        case WXK_F12:
            mkc = mplot::key::f12;
            break;

        case WXK_SHIFT:
            mkc = mplot::key::left_shift;
            break;
        case WXK_ALT:
            mkc = mplot::key::left_alt;
            break;
        case WXK_CONTROL:
            mkc = mplot::key::left_control;
            break;

        default:
            mkc = mplot::key::unknown;
            break;
        }
        return mkc;
    }

} // namespace
