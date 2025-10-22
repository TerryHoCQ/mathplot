#pragma once

/*!
 * \file Declares NormalsVisual to visualize the normals of another VisualModel
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
                sm::vec<float> pos = (*vp)[ii];
                sm::vec<float> nv = (*vn)[ii];

                sm::vec<float> end = pos + nv * scale_factor;
                sm::vec<float> arrow_line = nv * scale_factor;
                float len = arrow_line.length();
                sm::vec<float> cone_start = arrow_line.shorten (len * arrowhead_prop);
                cone_start += pos;
                std::array<float, 3> _clr = clr;
                if (!singlecolour) { _clr = (*vc)[ii]; }
                this->computeTube (pos, cone_start, _clr, _clr, thickness * scale_factor, shapesides);
                float conelen = (end - cone_start).length();
                if (arrow_line.length() > conelen) {
                    this->computeCone (cone_start, end, 0.0f, _clr, thickness * scale_factor * 2.0f, shapesides);
                }
            }

            std::array<uint32_t, 3> ti = {};
            // We also have vp1 (public) and triangles (also public)
            for (auto t : mymodel->triangles) {
                sm::vec<float, 3> nv = {};
                sm::vec<float, 3> nvc = {};
                sm::vec<float, 3> nvd = {};
                std::tie(ti, nv, nvc, nvd) = t;
                // Plot tn at mean location of ti
                sm::vec<float, 3> pos = mymodel->vp1[ti[0]] + mymodel->vp1[ti[1]] + mymodel->vp1[ti[2]];
                pos /= 3.0f;
                // Mesh triangle normals
                this->computeArrow (pos, (pos + nv * scale_factor), clr, thickness * scale_factor,
                                    arrowhead_prop, thickness * scale_factor * 2.0f, shapesides);
                // Computed triangle normals
                this->computeArrow (pos, (pos + nvc * scale_factor), clrnc, thickness * scale_factor,
                                    arrowhead_prop, thickness * scale_factor * 2.0f, shapesides);
                this->computeArrow (pos, (pos + nvd * scale_factor), clrnd, thickness * scale_factor,
                                    arrowhead_prop, thickness * scale_factor * 2.0f, shapesides);
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
        std::array<float, 3> clrnc = mplot::colour::grey60; // computed norm
        std::array<float, 3> clrnd = mplot::colour::grey90; // computed norm
    };

} // namespace mplot
