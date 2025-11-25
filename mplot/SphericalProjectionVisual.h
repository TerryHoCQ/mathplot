#pragma once

#include <array>
#include <vector>
#include <sm/mathconst>
#include <sm/vec>
#include <sm/range>
#include <sm/geometry>
#include <mplot/VisualModel.h>
#include <mplot/gl/version.h>

#define JC_VORONOI_IMPLEMENTATION
#include <mplot/jcvoronoi/jc_voronoi.h>

namespace mplot
{
    //! This class creates a flat projection of spherical data provided as vvecs of
    //! latitude-longitude pairs and scalar or vector values. Use VisualDataModel?
    template<typename T, int glver = mplot::gl::version_4_1> requires std::is_floating_point_v<T>
    class SphericalProjectionVisual : public mplot::VisualModel<glver>
    {
    public:
        SphericalProjectionVisual() {}
        SphericalProjectionVisual (const sm::vec<float, 3> _offset) : mplot::VisualModel<glver>(_offset) {}

        sm::geometry::spherical_projection::type proj_type = sm::geometry::spherical_projection::type::mercator;

        sm::vec<T, 2> project (const sm::vec<T, 2>& ll, const T radius)
        {
            if (this->proj_type == sm::geometry::spherical_projection::type::equirectangular) {
                return sm::geometry::spherical_projection::equirectangular (ll, radius);
            } else if (this->proj_type == sm::geometry::spherical_projection::type::cassini) {
                return sm::geometry::spherical_projection::cassini (ll, radius);
            } else {
                return sm::geometry::spherical_projection::mercator (ll, radius);
            }
        }

        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();
            this->xy.resize (this->latlong.size());
            for (uint32_t i = 0; i < this->latlong.size(); ++i) {
                this->xy[i] = this->project (this->latlong[i], this->radius).plus_one_dim();
            }
            this->voronoi2d();
        }

        //! Compute a triangle from 3 arbitrary corners
        void computeTriangle (sm::vec<T> c1, sm::vec<T> c2, sm::vec<T> c3, const std::array<float, 3>& colr)
        {
            // v is the face normal
            sm::vec<T> u1 = c1-c2;
            sm::vec<T> u2 = c2-c3;
            sm::vec<T> v = u1.cross(u2);
            v.renormalize();
            // Push corner vertices
            this->vertex_push (c1.as_float(), this->vertexPositions);
            this->vertex_push (c2.as_float(), this->vertexPositions);
            this->vertex_push (c3.as_float(), this->vertexPositions);
            // Colours/normals
            for (uint32_t i = 0; i < 3U; ++i) {
                this->vertex_push (colr, this->vertexColors);
                this->vertex_push (v.as_float(), this->vertexNormals);
            }
            this->indices.push_back (this->idx++);
            this->indices.push_back (this->idx++);
            this->indices.push_back (this->idx++);
        }

        void voronoi2d()
        {
            // Use mplot::range to find the extents of dataCoords. From these create a
            // rectangle to pass to jcv_diagram_generate.
            int ncoords = static_cast<int>(this->xy.size());
            sm::range<T> rx, ry;
            rx.search_init();
            ry.search_init();
            for (int i = 0; i < ncoords ; ++i) {
                if (std::isinf(this->xy[i][0])) {
                    std::cout << "xy["<<i<<"][0] is inf" << std::endl;
                }
                if (std::isinf(this->xy[i][1])) {
                    std::cout << "xy["<<i<<"][1] is inf" << std::endl;
                }
                rx.update (this->xy[i][0]);
                ry.update (this->xy[i][1]);
            }
            std::cout << "rx: " << rx << " ry: " << ry << std::endl;
            // Generate the 2D Voronoi diagram
            jcv_diagram diagram;
            std::memset (&diagram, 0, sizeof(jcv_diagram));
            jcv_rect domain = {
                jcv_point{rx.min - this->border_width, ry.min - this->border_width, 0.0f},
                jcv_point{rx.max + this->border_width, ry.max + this->border_width, 0.0f}
            };

            jcv_diagram_generate (ncoords, this->xy.data(), &domain, 0, &diagram);

            // We obtain access to the Voronoi cell sites:
            const jcv_site* sites = jcv_diagram_get_sites (&diagram);
            if (diagram.numsites != ncoords) {
                std::cout << "WARNING: diagram's ncoords (" << diagram.numsites << ") != ncoords (" << ncoords << ")?!?!\n";
            }

            for (int i = 0; i < diagram.numsites && i < ncoords; ++i) {
                const jcv_site* site = &sites[i];
                jcv_graphedge* e = site->edges; // The very first edge
                while (e) {
                    // Set z. Should be done in jcvoronoi, but haven't found out how
                    e->pos[0][2] = this->xy[i][2];
                    e->pos[1][2] = e->pos[0][2];
                    e = e->next;
                }
            }

            // To draw triangles iterate over the 'sites' and draw triangles
            for (int i = 0; i < diagram.numsites && i < ncoords; ++i) {
                const jcv_site* site = &sites[i];
                const jcv_graphedge* e = site->edges;
                std::array<float, 3> c = mplot::colour::black;
                if (static_cast<std::size_t>(site->index) < this->colour.size()) { c = this->colour[site->index]; }
                uint32_t site_triangles = 0;
                while (e) {
                    this->computeTriangle (site->p, e->pos[0], e->pos[1], c);
                    ++site_triangles;
                    e = e->next;
                }
            }
            // At end free the Voronoi diagram memory
            jcv_diagram_free (&diagram);
        }

        // latlong, supplied by user
        sm::vvec<sm::vec<T, 2>> latlong;
        // Colour, supplied by user
        sm::vvec<std::array<float, 3>> colour;
        // xy, result of projection, but in 3D
        sm::vvec<sm::vec<T, 3>> xy;
        // The radius of our sphere
        T radius = T{1};
        T border_width = std::numeric_limits<T>::epsilon();
    };
}
