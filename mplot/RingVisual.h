#pragma once

#include <array>
import sm.vec;
#include <mplot/VisualModel.h>

namespace mplot
{
    // A simple ring class that uses the primitive VisualModel::computeRing
    template <int glver = mplot::gl::version_4_1>
    struct RingVisual : public mplot::VisualModel<glver>
    {
        RingVisual(const sm::vec<float> _offset) : mplot::VisualModel<glver>::VisualModel (_offset) {}

        void initializeVertices()
        {
            this->computeRing (this->locn, this->clr, this->radius, this->thickness, this->segments);
        }

        std::array<float, 3> clr = mplot::colour::goldenrod;
        sm::vec<float, 3> locn = { 0.0f };
        float radius = 1.0f;
        float thickness = 0.2f;
        int segments = 80;
    };
} // namespace
