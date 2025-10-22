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

            const float cone_r = this->thickness * this->scale_factor * 2.0f;
            const float tube_r = this->thickness * this->scale_factor;
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
                // (*vp)[ii] is position, (*vn)[ii] is normal
                std::array<float, 3> _clr = clr;
                if (!singlecolour) { _clr = (*vc)[ii]; }
                this->computeArrow ((*vp)[ii], ((*vp)[ii] + (*vn)[ii] * this->scale_factor),
                                    _clr, tube_r, this->arrowhead_prop, cone_r, this->shapesides);
            }

            std::array<uint32_t, 3> ti = {};
            sm::vec<float, 3> nv = {};
            sm::vec<float, 3> nvc = {};
            sm::vec<float, 3> nvd = {};
            sm::vec<float, 3> pos = {};
            // We also have vp1 (public) and triangles (also public)
            for (auto t : mymodel->triangles) {
                std::tie(ti, nv, nvc, nvd) = t;
                // Plot tn at mean location of ti
                pos = (mymodel->vp1[ti[0]] + mymodel->vp1[ti[1]] + mymodel->vp1[ti[2]]) / 3.0f;
                // Mesh triangle normals
                this->computeArrow (pos, (pos + nv * this->scale_factor),
                                    clr, tube_r, this->arrowhead_prop, cone_r, this->shapesides);
                // Computed triangle normals
                this->computeArrow (pos, (pos + nvc * this->scale_factor),
                                    clrnc, tube_r, this->arrowhead_prop, cone_r, this->shapesides);
                this->computeArrow (pos, (pos + nvd * scale_factor),
                                    clrnd, tube_r, this->arrowhead_prop, cone_r, this->shapesides);
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
