#pragma once

/*!
 * \file Declares ConeVisual to visualize a simple cone
 */

#include <array>
#include <sm/vec>
#include <mplot/colour.h>
#include <mplot/VisualModel.h>

namespace mplot {

    //! A class to visualize a single vector
    template <int glver = mplot::gl::version_4_1>
    struct ConeVisual : public VisualModel<glver>
    {
        ConeVisual (const sm::vec<float> _offset) { this->viewmatrix.translate (_offset); }

        //! Do the computations to initialize the vertices that will represent the Quivers.
        void initializeVertices()
        {
            this->computeCone (this->start, this->end, this->ringoffset, this->clr, this->radius, this->shapesides);
        }

        sm::vec<float> clr = {1.0f, 0.0f, 0.7f};
        sm::vec<float> start = {0,0,0};
        sm::vec<float> end = {1,0,0};
        float radius = 0.3f;
        float ringoffset = 0.0f;

        // How many sides to an arrow/cone/sphere? Increase for smoother arrow
        // objects. Decrease to ease the load on your CPU and GPU. 12 is a reasonable
        // compromise. You can set this before calling finalize().
        int shapesides = 12;
    };

} // namespace mplot
