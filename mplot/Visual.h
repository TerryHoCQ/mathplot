/*!
 * \file
 *
 * Awesome graphics code for high performance graphing and visualisation.
 *
 * This is the main visual scene class in mathplot and derives from mplot::VisualNoMX, which
 * derives from VisualOwnableMX, adding window handling with GLFW3.
 *
 * Created by Seb James on 2019/05/01
 *
 * \author Seb James
 * \date May 2019
 */
#pragma once

#include <mplot/VisualMX.h>

namespace mplot {

    /*!
     * Visual 'scene' class
     *
     * A class for visualising computational models on an OpenGL screen.
     *
     * Each Visual will have its own GLFW window and is essentially a "scene" containing a number of
     * objects. One object might be the visualisation of some data expressed over a HexGrid. Another
     * could be a GraphVisual object. The class handles mouse events to allow the user to rotate and
     * translate the scene, as well as use keys to generate particular effects/views.
     *
     * The implementation code for mplot::Visual is actually provided by the multi-context aware
     * mplot::VisualMX class, which loads GLAD headers with the multi context option MX enabled.
     *
     * If you want a mplot::Visual which loads a single set of OpenGL function aliases such as
     * glClear, glEnable, glDisable and so on, you can use mplot::VisualNoMX<>. If you want to be
     * explicit about the fact that you're using the multi-context aware class, use
     * mplot::VisualMX<>
     *
     * \tparam glver The OpenGL version, encoded as a single int (see mplot::gl::version)
     */
    template <int glver = mplot::gl::version_4_1>
    struct Visual : public mplot::VisualMX<glver>
    {
        Visual (const int _width, const int _height, const std::string& _title, const bool _version_stdout = true)
            : mplot::VisualMX<glver> (_width, _height, _title, _version_stdout) {}
    };

} // namespace mplot
