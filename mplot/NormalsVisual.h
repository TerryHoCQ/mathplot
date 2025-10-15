#pragma once

/*!
 * \file Declares VectorVisual to visualize the normals of another VisualModel
 */

#include <array>
#include <sm/vec>
#include <mplot/colour.h>
#include <mplot/VisualModel.h>

namespace mplot {

    //! A class to visualize normals for another model
    template <int glver = mplot::gl::version_4_1>
    class NormalsVisual : public VisualModel<glver>
    {
    public:
        NormalsVisual(mplot::VisualModel<glver>* _mymodel)
        {
            this->mymodel = _mymodel;
            this->viewmatrix = _mymodel->getViewMatrix();
        }

        void initializeVertices()
        {
            if (mymodel == nullptr) {
                std::cout << "NormalsVisual: I have no model; returning\n";
                return;
            }

            // Copy data out of my model...
            std::vector<float> mymodelPositions = mymodel->getVertexPositions();
            std::vector<float> mymodelNormals = mymodel->getVertexNormals();
            std::vector<float> mymodelColors = {};
            if (!singlecolour) { mymodelColors = mymodel->getVertexColors(); }
            // And interpret it
            auto vp = reinterpret_cast<const std::vector<sm::vec<float, 3>>*>(&mymodelPositions);
            auto vn = reinterpret_cast<const std::vector<sm::vec<float, 3>>*>(&mymodelNormals);
            auto vc = reinterpret_cast<const std::vector<std::array<float, 3>>*>(&mymodelColors);

            for (uint32_t ii = 0; ii < vn->size(); ++ii) {
                sm::vec<float> end = (*vp)[ii] + (*vn)[ii] * scale_factor;
                sm::vec<float> arrow_line = (*vn)[ii] * scale_factor;
                float len = arrow_line.length();
                sm::vec<float> cone_start = arrow_line.shorten (len * arrowhead_prop);
                cone_start += (*vp)[ii];
                std::array<float, 3> _clr = clr;
                if (!singlecolour) { _clr = (*vc)[ii]; }
                this->computeTube ((*vp)[ii], cone_start, _clr, _clr, thickness * scale_factor, shapesides);
                float conelen = (end - cone_start).length();
                if (arrow_line.length() > conelen) {
                    this->computeCone (cone_start, end, 0.0f, _clr, thickness * scale_factor * 2.0f, shapesides);
                }
            }
        };

        // The model for which we will plot normal vectors
        mplot::VisualModel<glver>* mymodel = nullptr;
        // How many sides to each normal vector
        int shapesides = 12;
        // thickness for the normal vectors
        float thickness = 0.025f;
        // What proportion of the arrow length should the arrowhead length be?
        float arrowhead_prop = 0.25f;
        // How much to linearly scale the size of the vector
        float scale_factor = 0.1f;
        // Vector single colour
        bool singlecolour = false;
        std::array<float, 3> clr = mplot::colour::grey20;
    };

} // namespace mplot
