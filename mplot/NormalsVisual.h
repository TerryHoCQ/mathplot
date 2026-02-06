#pragma once

/*!
 * \file Declares NormalsVisual to visualize the normals of another VisualModel
 */

#include <array>
#include <sm/vec>
#include <sm/flags>
#include <mplot/colour.h>
#include <mplot/VisualModel.h>

namespace mplot
{
    enum class normalsvisual_flags : uint32_t
    {
        show_gl_normals,  // Show the OpenGL vertex normals?
        show_tri_edges,   // Show the OpenGLtriangle edge vectors?
        show_tri_normals, // Show the OpenGL triangle-derived normal?
        show_halfedges,   // Show the navmesh halfedges (all of them)?
        show_inner_halfedges,    // Show the main, internal navmesh halfedges (the blue ones)?
        show_boundary_halfedges, // Show the boundary navmesh halfedges (the red ones)?
        singlecolour      // Plot vertex normals in a single colour?
    };

    //! A class to visualize normals for another model
    template <int glver = mplot::gl::version_4_1>
    class NormalsVisual : public VisualModel<glver>
    {
    public:
        NormalsVisual(mplot::VisualModel<glver>* _mymodel)
        {
            this->options_defaults();
            this->mymodel = _mymodel;
            this->viewmatrix = _mymodel->getViewMatrix();
            // We create the model's navmesh, in case it wasn't already done
            this->mymodel->make_navmesh();
        }

        void initializeVertices()
        {
            if (mymodel == nullptr) {
                std::cout << "NormalsVisual: I have no model; returning\n";
                return;
            }
            std::cout << "InitializeVertices\n";
            const float cone_r = this->thickness * this->scale_factor * 2.0f;
            const float tube_r = this->thickness * this->scale_factor;
            // Copy data out of my model...
            std::vector<float> mymodelPositions = mymodel->getVertexPositions();
            std::vector<float> mymodelNormals = mymodel->getVertexNormals();
            std::vector<float> mymodelColors = {};
            if (this->options.test (normalsvisual_flags::singlecolour) == false) {
                mymodelColors = mymodel->getVertexColors();
            }
            // And interpret it
            auto vp = reinterpret_cast<const std::vector<sm::vec<float, 3>>*>(&mymodelPositions);
            auto vn = reinterpret_cast<const std::vector<sm::vec<float, 3>>*>(&mymodelNormals);
            auto vc = reinterpret_cast<const std::vector<std::array<float, 3>>*>(&mymodelColors);

            if (this->options.test (normalsvisual_flags::show_gl_normals)) {
                std::cout << "Showing " << vn->size() << " OpenGL vertex normals" << std::endl;
                for (uint32_t ii = 0; ii < vn->size(); ++ii) {
                    // (*vp)[ii] is position, (*vn)[ii] is normal
                    std::array<float, 3> _clr = clr;
                    if (this->options.test (normalsvisual_flags::singlecolour) == false) { _clr = (*vc)[ii]; }
                    this->computeArrow ((*vp)[ii], ((*vp)[ii] + (*vn)[ii] * this->scale_factor),
                                        _clr, tube_r, this->arrowhead_prop, cone_r, this->shapesides);
                }
            }
            // If we also have the navmesh, then use its triangles to show face normals
            if (mymodel->navmesh) {

                sm::vec<float, 3> nv = {};
                sm::vec<float, 3> nvc = {};
                sm::vec<float, 3> nvd = {};
                sm::vec<float, 3> pos = {};

                if (this->options.any_of ({normalsvisual_flags::show_tri_normals, normalsvisual_flags::show_tri_edges})) {
                    std::cout << "About to show normals and/or edges for " << mymodel->navmesh->triangles.size() << " triangles" << std::endl;

                    for (auto t : mymodel->navmesh->triangles) {

                        sm::vec<sm::vec<float>, 3> vrts = mymodel->navmesh->triangle_vertices (t.hi);
                        // Plot normal/edge vectors at mean location of f
                        pos = vrts.mean();
                        if (this->options.test (normalsvisual_flags::show_tri_normals)) {
                            nv = mymodel->navmesh->triangle_normal (vrts);
                            // Mesh triangle normals
                            this->computeArrow (pos, (pos + nv * this->scale_factor),
                                                clr, tube_r, this->arrowhead_prop, cone_r, this->shapesides);
                        }
                        if (this->options.test (normalsvisual_flags::show_tri_edges)) {
                            // Computed triangle edges
                            nvc = vrts[1] - vrts[0];
                            nvd = vrts[2] - vrts[0];
                            this->computeArrow (pos, (pos + nvc * this->scale_factor),
                                                clrnc, tube_r, this->arrowhead_prop, cone_r, this->shapesides);
                            this->computeArrow (pos, (pos + nvd * scale_factor),
                                                clrnd, tube_r, this->arrowhead_prop, cone_r, this->shapesides);
                        }
                    }
                }

                // Can also show halfedges from the navmesh
                if (this->options.any_of ({normalsvisual_flags::show_halfedges,
                                           normalsvisual_flags::show_inner_halfedges,
                                           normalsvisual_flags::show_boundary_halfedges})) {

                    if (this->options.test (normalsvisual_flags::show_halfedges)
                        || this->options.test ({normalsvisual_flags::show_inner_halfedges, normalsvisual_flags::show_boundary_halfedges})) {
                        std::cout << "About to show " << mymodel->navmesh->halfedge.size() << " halfedges from the model...\n";
                    } else {
                        if (this->options.test (normalsvisual_flags::show_inner_halfedges)) {
                            std::cout << "Showing only inner halfedges (approx " << mymodel->navmesh->halfedge.size() << ")\n";
                        } else if (this->options.test (normalsvisual_flags::show_inner_halfedges)) {
                            std::cout << "Showing only boundary halfedges (approx " << mymodel->navmesh->halfedge.size() << ")\n";
                        }
                    }

                    for (auto h : mymodel->navmesh->halfedge) {

                        auto p0 = mymodel->navmesh->vertex[h.vi[0]].p;
                        auto p1 = mymodel->navmesh->vertex[h.vi[1]].p;
                        if ((h.flags & 0x2) == 0x2) {
                            // special/rogue
                            std::cout << "Showing a rogue!\n";
                            if (this->options.any_of ({normalsvisual_flags::show_halfedges, normalsvisual_flags::show_boundary_halfedges, normalsvisual_flags::show_inner_halfedges})) {

                                this->computeArrow (p0, p1, mplot::colour::yellow,
                                                    tube_r, this->arrowhead_prop, cone_r, this->shapesides);
                            }
                        } else if ((h.flags & 0x1) == 0x1) {
                            // boundary
                            if (this->options.any_of ({normalsvisual_flags::show_halfedges, normalsvisual_flags::show_boundary_halfedges})) {

                                this->computeArrow (p0, p1, mplot::colour::crimson,
                                                    tube_r, this->arrowhead_prop, cone_r, this->shapesides);
                            }
                        } else {
                            // internal
                            if (this->options.any_of ({normalsvisual_flags::show_halfedges, normalsvisual_flags::show_inner_halfedges})) {
                                this->computeArrow (p0, p1, mplot::colour::dodgerblue2,
                                                    tube_r, this->arrowhead_prop, cone_r, this->shapesides);
                            }
                        }
                    }
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
        // Options for this VisualModel. Set these with calls like
        // vm.options.set (mplot::normalsvisual_flags::show_gl_normals, false)
        // from your client code
        sm::flags<normalsvisual_flags> options;
        void options_defaults()
        {
            this->options.reset();
            this->options.set (normalsvisual_flags::show_gl_normals, true);
            this->options.set (normalsvisual_flags::show_tri_edges, false);
            this->options.set (normalsvisual_flags::show_tri_normals, true);
            this->options.set (normalsvisual_flags::show_halfedges, false);
            this->options.set (normalsvisual_flags::singlecolour, false);
        }
        // Vector single colour
        std::array<float, 3> clr = mplot::colour::grey20;
        std::array<float, 3> clrnc = mplot::colour::grey60; // computed norm
        std::array<float, 3> clrnd = mplot::colour::grey90; // computed norm
    };

} // namespace mplot
