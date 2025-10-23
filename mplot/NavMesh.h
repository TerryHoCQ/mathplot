/*!
 * \file
 *
 * A navigation mesh class. An instance of this navigation mesh may be owned by a VisualModel to aid
 * navigation across and around the model.
 *
 * \author Seb James
 * \date October 2025
 */

#pragma once

#include <cstdint>
#include <limits>
#include <tuple>
#include <array>
#include <vector>
#include <set>
#include <map>

#include <sm/vec>
#include <sm/vvec>

namespace mplot
{
    /*!
     * Navigation mesh of triangles.
     *
     * This is built from an OpenGL vertex/indices set by VisualModel::make_navmesh()
     */
    struct NavMesh
    {
        /*!
         * Minimum set of vertices to generate a topological mesh. populated by
         * VisualModel::make_navmesh()
         */
        std::vector<sm::vec<float, 3>> vertex;

        /*!
         * The edges that make up the same triangles as are shown with the parent VisualModel's
         * indices & vertexPositions, but in terms of this->vertex.  Each edge must be two indices
         * in *ascending numerical order*. populated by VisualModel::make_navmesh()
         */
        std::set<std::array<uint32_t, 2>> edges;

        /*!
         * Triangles too. Might be more useful than edges. Triangle given as indices into
         * this->vertex. populated by VisualModel::make_navmesh()
         */
        sm::vvec<std::tuple<std::array<uint32_t, 3>, sm::vec<float, 3>, sm::vec<float, 3>, sm::vec<float, 3>>> triangles;

        /*!
         * Maps index in vertex to the original parent->indices index. populated by
         * VisualModel::make_navmesh()
         */
        sm::vvec<sm::vvec<uint32_t>> vertexidx_to_indices;

        /*!
         * Return index of this->vertex that is closest to scene_coord. Can use vertexidx_to_indices
         * to find the indices into vertexPositions and vertexNormals that this index in the
         * topographic mesh relates to.
         *
         * \param scene_coord Supplied coordinate in scene frame of referencea
         * \param viewmatrix The viewmatrix of the model which converts model frame coordinates to the scene frame
         */
        uint32_t find_vertex_nearest (const sm::vec<float, 3>& scene_coord, const sm::mat44<float>& viewmatrix) const
        {
            uint32_t i = std::numeric_limits<uint32_t>::max();
            // Brute force it. (But we have a mesh; can this guarantee a faster search? I don't think so)
            float min_d = std::numeric_limits<float>::max();
            for (uint32_t j = 0; j < this->vertex.size(); ++j) {
                sm::vec<> vcoord = (viewmatrix * this->vertex[j]).less_one_dim();
                float d = (scene_coord - vcoord).length();
                if (d < min_d) {
                    min_d = d;
                    i = j;
                }
            }
            return i;
        }

        sm::vec<sm::vec<float, 3>, 3> triangle_vertices (const std::array<uint32_t, 3>& tri_indices) const
        {
            sm::vec<sm::vec<float, 3>, 3> trivert;
            if (tri_indices[0] < this->vertex.size()) { trivert[0] = this->vertex[tri_indices[0]]; }
            if (tri_indices[1] < this->vertex.size()) { trivert[1] = this->vertex[tri_indices[1]]; }
            if (tri_indices[2] < this->vertex.size()) { trivert[2] = this->vertex[tri_indices[2]]; }
            return trivert;
        }

        sm::vvec<uint32_t> neighbours (const uint32_t _idx) const
        {
            sm::vvec<uint32_t> rtn;
            // Search edges to find those that include _idx and then pack up the other ends in a return object
            for (auto e : this->edges) {
                // we have e[0] and e[1]
                if (e[0] == _idx) {
                    // neighb is e[1]
                    rtn.push_back (e[1]);
                } else if (e[1] == _idx) {
                    // neighb is e[0]
                    rtn.push_back (e[0]);
                }
            }
            return rtn;
        }

        sm::vvec<std::array<uint32_t, 3>> neighbour_triangles (const uint32_t _idx) const
        {
            sm::vvec<std::array<uint32_t, 3>> rtn;
            for (auto t: this->triangles) {
                auto [ti, tn, tnc, tnd] = t;
                // If it includes _idx, add it to rtn
                if (ti[0] == _idx || ti[1] == _idx || ti[2] == _idx) {
                    rtn.push_back (ti);
                }
            }
            return rtn;
        }

        // Return a tuple containing crossing location, triangle identity (three indices) and triangle normal vector
        std::tuple<sm::vec<float, 3>, std::array<uint32_t, 3>, sm::vec<float, 3>>
        find_triangle_crossing (const sm::vec<float, 3>& coord, const sm::vec<float, 3>& vdir) const
        {
            for (auto tri : triangles) {
                auto [ti, tn, tnc, tnd] = tri;
                auto [isect, p] = sm::algo::ray_tri_intersection<float> (this->vertex[ti[0]], this->vertex[ti[1]], this->vertex[ti[2]], coord - (vdir / 2.0f), vdir);
                if (isect) { return {p, ti, tn}; }
            }

            // Failed to find, return container full of maxes
            sm::vec<float, 3> p = {};
            p.set_from (std::numeric_limits<float>::max());
            constexpr uint32_t umax = std::numeric_limits<uint32_t>::max();
            return {p , std::array<uint32_t, 3>{umax, umax, umax}, p};

        }

        // Find a triangle containing indices a and b that isn't 'not_this' and return, along with its normal.
        std::tuple<std::array<uint32_t, 3>, sm::vec<float>>
        find_other_triangle_containing (const uint32_t a, const uint32_t b, const std::array<uint32_t, 3>& not_this) const
        {
            constexpr bool debug_normals = false;

            constexpr uint32_t umax = std::numeric_limits<uint32_t>::max();
            std::array<uint32_t, 3> other = {umax, umax, umax};
            constexpr float fmax = std::numeric_limits<float>::max();
            sm::vec<float> other_n = {fmax, fmax, fmax};
            sm::vec<float> my_n = {fmax, fmax, fmax}; // debug
            for (auto tri : triangles) {
                auto [ti, tn, tnc, tnd] = tri;
                if (ti == not_this) {
                    if constexpr (debug_normals) { my_n = tn; }
                    continue;
                }
                if ((ti[0] == a && (ti[1] == b || ti[2] == b))
                    || (ti[1] == a && (ti[0] == b || ti[2] == b))
                    || (ti[2] == a && (ti[0] == b || ti[1] == b))) {
                    other = ti;
                    other_n = tn;
                    if constexpr (!debug_normals) { break; }
                }
            }
            if constexpr (debug_normals) {
                std::cout << "my_n: " << my_n << " and other_n: " << other_n << std::endl;
            }
            return {other, other_n};
        }

    }; // struct NavMesh

} // namespace
