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
#include <stdexcept>
#include <string_view>
#include <unordered_map>

#include <sm/vec>
#include <sm/vvec>
#include <sm/flags>
#include <sm/mat>
#include <sm/geometry>

namespace mplot
{
    // Exception that returns triangles that were near the location of the error
    struct NavException : public std::exception
    {
        enum class type : uint32_t { generic, no_intersection, zero_mv, mv_to_vertex, undetected_crossing, nan_mv, off_edge };

        NavException (const type _type) : m_type(_type) {}
        NavException (const type _type, const std::vector<std::array<uint32_t, 4>>& t) : m_type(_type) { this->tris = t; }

        using std::exception::what;
        const char* what()
        {
            switch (m_type) {
            case type::no_intersection:
                return "No intersection (at start) with triangle or neighbours";
                break;
            case type::zero_mv:
                return "Zero length mv_inplane so stop/freeze/crash";
                break;
            case type::mv_to_vertex:
                return "We've moved to a vertex, should have captured this case";
                break;
            case type::undetected_crossing:
                return "Should have detected crossing just now";
                break;
            case type::nan_mv:
                return "mv_inplane contained NaN";
                break;
            case type::off_edge:
                return "The movement went off the edge of the model";
                break;
            case type::generic:
            default:
                break;
            }
            return "Generic";
        }
        // Error type determines message generated
        type m_type = type::generic;
        // Triangles of interest (as indices into NavMesh::vertex)
        std::vector<std::array<uint32_t, 4>> tris;
    };

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
        std::vector<sm::vec<float>> vertex;

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
        sm::vvec<std::tuple<std::array<uint32_t, 4>, sm::vec<float>, sm::vec<float>, sm::vec<float>>> triangles;

        /*!
         * For triangles[i], one_neighbours[i] should contain the indices of the triangles that are
         * its one-vertex-shared-neighbours
         */
        std::unordered_map<uint32_t, sm::vvec<uint32_t>> one_neighbours;

        /*!
         * For triangles[i], two_neighbours[i] should contain the indices of the triangles that are
         * its two-vertices-shared-neighbours. Could be sm::vec<uint32_t, 3> as can never be >3?
         */
        std::unordered_map<uint32_t, sm::vvec<uint32_t>> two_neighbours;

        /*!
         * Maps index in vertex to the original parent->indices index. populated by
         * VisualModel::make_navmesh()
         */
        sm::vvec<sm::vvec<uint32_t>> vertexidx_to_indices;

        //! Holds a copy of the bb of the parent model
        sm::range<sm::vec<float>> bb;

        //! When navigating, this is the 'current triangle' that you're located over/near
        std::array<uint32_t, 4> ti0 = {};

        /*!
         * The normal of ti0. This is the current triangle normal (in our mesh's frame of
         * reference) that our agent/camera is 'next to'
         */
        sm::vec<float> tn0 = {};

        /*!
         * Stabilisation flag: if true, no rotation is applied when moving over a triangle boundary
         * in NavMesh::compute_mesh_movement. If false, then a rotation about the triangle boundary
         * is made.
         */
        bool stabilised = false;

        /*!
         * Return index of this->vertex that is closest to scene_coord. Can use vertexidx_to_indices
         * to find the indices into vertexPositions and vertexNormals that this index in the
         * topographic mesh relates to.
         *
         * \param scene_coord Supplied coordinate in scene frame of referencea
         * \param viewmatrix The viewmatrix of the model which converts model frame coordinates to the scene frame
         */
        uint32_t find_vertex_nearest (const sm::vec<float>& scene_coord, const sm::mat<float, 4>& viewmatrix) const
        {
            uint32_t i = std::numeric_limits<uint32_t>::max();
            // Brute force it. (But we have a mesh; can this guarantee a faster search? I don't think so)
            float min_d = std::numeric_limits<float>::max();
            for (uint32_t j = 0; j < this->vertex.size(); ++j) {
                sm::vec<float> vcoord = (viewmatrix * this->vertex[j]).less_one_dim();
                float d = (scene_coord - vcoord).length();
                if (d < min_d) {
                    min_d = d;
                    i = j;
                }
            }
            return i;
        }

        // Return the three vertices for the triangle specified as three indices into NavMesh::vertex
        sm::vec<sm::vec<float>, 3> triangle_vertices (const std::array<uint32_t, 4>& tri_indices) const
        {
            sm::vec<sm::vec<float>, 3> trivert;
            if (tri_indices[0] < this->vertex.size()) { trivert[0] = this->vertex[tri_indices[0]]; }
            if (tri_indices[1] < this->vertex.size()) { trivert[1] = this->vertex[tri_indices[1]]; }
            if (tri_indices[2] < this->vertex.size()) { trivert[2] = this->vertex[tri_indices[2]]; }
            return trivert;
        }

        // Return the three vertices for the triangle specified as three indices into NavMesh::vertex transformed by transform
        sm::vec<sm::vec<float>, 3> triangle_vertices (const std::array<uint32_t, 4>& tri_indices, const sm::mat<float, 4>& transform) const
        {
            sm::vec<sm::vec<float>, 3> trivert;
            if (tri_indices[0] < this->vertex.size()) { trivert[0] = (transform * this->vertex[tri_indices[0]]).less_one_dim(); }
            if (tri_indices[1] < this->vertex.size()) { trivert[1] = (transform * this->vertex[tri_indices[1]]).less_one_dim(); }
            if (tri_indices[2] < this->vertex.size()) { trivert[2] = (transform * this->vertex[tri_indices[2]]).less_one_dim(); }
            return trivert;
        }

        // Compute the triangle normal for the ordered triplet of triangle vertices, tverts
        sm::vec<float, 3> triangle_normal (const sm::vec<sm::vec<float>, 3>& tverts) const
        {
            sm::vec<float> n = (tverts[1] - tverts[0]).cross (tverts[2] - tverts[0]);
            n.renormalize();
            return n;
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

#if 0 // unused functions

        // Determine if ti0 is on the edge of the model (with < 3 edge neighbours), If so, place 1
        // in its final element. Also mark as on edge any nighbours sharing one of its vertices
        uint32_t mark_if_on_edge (std::array<uint32_t, 4>& _ti0)
        {
            constexpr bool debug_met = false;
            uint32_t n2 = 0; // Neighbours sharing 2 vertices (up to 3)

            std::vector<std::array<uint32_t, 4>*> neighb_edge_tris;

            for (auto& t: this->triangles) {
                auto [ti, tn, tnc, tnd] = t;
                auto a0 = _ti0[0];
                auto b0 = _ti0[1];
                auto c0 = _ti0[2];
                auto a = ti[0];
                auto b = ti[1];
                auto c = ti[2];
                if ((   a == a0 && ((b == b0 && c != c0) || (c == b0 && b != c0)))
                    || (b == a0 && ((a == b0 && c != c0) || (c == b0 && a != c0)))
                    || (c == a0 && ((a == b0 && b != c0) || (b == b0 && a != c0)))) {
                    ++n2;
                    neighb_edge_tris.push_back (&ti);
                }
                else if ((   a == b0 && ((b == c0 && c != a0) || (c == c0 && b != a0)))
                         || (b == b0 && ((a == c0 && c != a0) || (c == c0 && a != a0)))
                         || (c == b0 && ((a == c0 && b != a0) || (b == c0 && a != a0)))) {
                    ++n2;
                    neighb_edge_tris.push_back (&ti);
                }
                else if ((   a == c0 && ((b == a0 && c != b0) || (c == a0 && b != b0)))
                         || (b == c0 && ((a == a0 && c != b0) || (c == a0 && a != b0)))
                         || (c == c0 && ((a == a0 && b != b0) || (b == a0 && a != b0)))) {
                    ++n2;
                    neighb_edge_tris.push_back (&ti);
                }
            }

            if (n2 < 3) {
                if constexpr (debug_met) {
                    std::cout << _ti0[0] << "-" << _ti0[1] << "-" << _ti0[2] << " is on the edge";
                }
                _ti0[3] = 1;
                for (auto& net : neighb_edge_tris) {
                    if constexpr (debug_met) { std::cout << " mark vtx neighbour "; }
                    (*net)[3] = 1;
                }
                if constexpr (debug_met) { std::cout << std::endl; }

                return neighb_edge_tris.size() + 1;
            } // Meaning that the triangle is 'on the edge' of the model
            return 0;
        }

        // Go through all triangles, marking if they're an 'edge' triangle. A triangle is ALSO on
        // the edge if on of its neighbours has < 3 edge neighbours.
        void mark_edge_triangles()
        {
            constexpr bool debug_met = false;
            uint32_t ec = 0;
            for (auto& t: this->triangles) {
                auto& [ti, tn, tnc, tnd] = t;
                ec += mark_if_on_edge (ti); // ALSO loops through triangles
            }
            if constexpr (debug_met) {
                std::cout << ec << " / " << this->triangles.size() << " triangles are on edge\n";
            }
        }

        // Count 2-vertex (i.e. edge) neighbours and also 1-vertex neighbours for triangle _ti0
        std::tuple<uint32_t, uint32_t> count_neighbour_triangles (const std::array<uint32_t, 4>& _ti0) const
        {
            // Count neighbour triangles
            uint32_t n1 = 0; // Neighbour sharing 1 vertex (any number)
            uint32_t n2 = 0; // Neighbours sharing 2 vertices (up to 3)
            for (auto t: this->triangles) {
                auto [ti, tn, tnc, tnd] = t;
                auto a0 = _ti0[0];
                auto b0 = _ti0[1];
                auto c0 = _ti0[2];
                auto a = ti[0];
                auto b = ti[1];
                auto c = ti[2];

                if ((   a == a0 && ((b == b0 && c != c0) || (c == b0 && b != c0)))
                    || (b == a0 && ((a == b0 && c != c0) || (c == b0 && a != c0)))
                    || (c == a0 && ((a == b0 && b != c0) || (b == b0 && a != c0)))) { ++n2; }

                else if ((   a == b0 && ((b == c0 && c != a0) || (c == c0 && b != a0)))
                         || (b == b0 && ((a == c0 && c != a0) || (c == c0 && a != a0)))
                         || (c == b0 && ((a == c0 && b != a0) || (b == c0 && a != a0)))) { ++n2; }

                else if ((   a == c0 && ((b == a0 && c != b0) || (c == a0 && b != b0)))
                         || (b == c0 && ((a == a0 && c != b0) || (c == a0 && a != b0)))
                         || (c == c0 && ((a == a0 && b != b0) || (b == a0 && a != b0)))) { ++n2; }

                else if ((   a == a0 && b != b0 && b != c0 && c != b0 && c != c0)
                         || (b == a0 && c != b0 && c != c0 && a != b0 && a != c0)
                         || (c == a0 && a != b0 && a != c0 && b != b0 && b != c0)) { ++n1; }
            }

            return {n2, n1};
        }

        sm::vvec<std::array<uint32_t, 4>> neighbour_triangles (const uint32_t _idx) const
        {
            sm::vvec<std::array<uint32_t, 4>> rtn;
            for (auto t: this->triangles) {
                auto [ti, tn, tnc, tnd] = t;
                // If it includes _idx, add it to rtn
                if (ti[0] == _idx || ti[1] == _idx || ti[2] == _idx) {
                    rtn.push_back (ti);
                }
            }
            return rtn;
        }
#endif
        // Find all the neighbours of triangle *vertex* index a.
        // \return tuple containing triangle vertex indices and triangle normal.
        std::vector<std::tuple<std::array<uint32_t, 4>, sm::vec<float>>>
        find_neighbours (const uint32_t a) const
        {
            std::vector<std::tuple<std::array<uint32_t, 4>, sm::vec<float>>> rtn = {};
            for (auto tri : triangles) {
                auto [ti, tn, tnc, tnd] = tri;
                if (ti[0] == a || ti[1] == a || ti[2] == a) { rtn.push_back({ti, tn}); }
            }
            return rtn;
        }

        // Find all the one-neighbours of 'of_this'
        std::vector<std::tuple<std::array<uint32_t, 4>, sm::vec<float>>>
        find_one_neighbours (const std::array<uint32_t, 4>& of_this) const
        {
            std::vector<std::tuple<std::array<uint32_t, 4>, sm::vec<float>>> rtn = {};
            auto a = of_this[0];
            auto b = of_this[1];
            auto c = of_this[2];
            for (auto tri : triangles) {
                auto [ti, tn, tnc, tnd] = tri;
                if ((ti[0] == a && ti[1] != b && ti[1] != c && ti[2] != b && ti[2] != c)
                    || (ti[1] == a && ti[2] != b && ti[2] != c && ti[0] != b && ti[0] != c)
                    || (ti[2] == a && ti[0] != b && ti[0] != c && ti[1] != b && ti[1] != c)
                    ||
                    (ti[0] == b && ti[1] != c && ti[1] != a && ti[2] != c && ti[2] != a)
                    || (ti[1] == b && ti[2] != c && ti[2] != a && ti[0] != c && ti[0] != a)
                    || (ti[2] == b && ti[0] != c && ti[0] != a && ti[1] != c && ti[1] != a)
                    ||
                    (ti[0] == c && ti[1] != a && ti[1] != b && ti[2] != a && ti[2] != b)
                    || (ti[1] == c && ti[2] != a && ti[2] != b && ti[0] != a && ti[0] != b)
                    || (ti[2] == c && ti[0] != a && ti[0] != b && ti[1] != a && ti[1] != b)) {

                    rtn.push_back ({ti, tn});
                }
            }
            return rtn;
        }

        /*
         * Populate containers of neighbour relations between the triangles. That's
         * this->one_neighbours and this->two_neighbours. Can I do this in a non-n^2 way?
         * The key is the half-edge data structure.
         * See: https://jerryyin.info/geometry-processing-algorithms/half-edge/
         */
        void compute_neighbour_relations()
        {
#if 0
            for (auto tri : triangles) {
                auto [ti, tn, tnc, tnd] = tri;
                find_one_neighbours() // but returning index. This loops through all triangles.
            }
#endif
        }

        std::tuple<std::array<uint32_t, 4>, sm::vec<float>>
        first_triangle_containing (uint32_t _idx) const
        {
            for (auto t: this->triangles) {
                auto [ti, tn, tnc, tnd] = t;
                if (ti[0] == _idx || ti[1] == _idx || ti[2] == _idx) {
                    return {ti, tn};
                }
            }
            return {};
        }

        /*
         * Find the location, and the triangle indices at which a ray starting from coord (scene
         * frame) with direction vdir - the 'penetration point' intersects with this NavMesh
         * model. The length of vdir is used to avoid finding the intersection at the 'back' of the
         * model.
         *
         * \param ti_ml The most likely triangle, if you know what it probably is, to reduce the
         * search time.
         *
         * \return a tuple containing crossing location, triangle identity (three indices) and triangle normal vector
         */
        std::tuple<sm::vec<float>, std::array<uint32_t, 4>, sm::vec<float>>
        find_triangle_crossing (const sm::vec<float>& coord_mf, const sm::vec<float>& vdir,
                                const std::array<uint32_t, 4> ti_ml = {std::numeric_limits<uint32_t>::max()}) const
        {
            constexpr auto umax = std::numeric_limits<uint32_t>::max();
            constexpr auto fmax = std::numeric_limits<float>::max();
            sm::vec<float> vstart = coord_mf - (vdir / 2.0f);

            // Return objects
            sm::vec<float> isect_p = { fmax, fmax, fmax };
            std::array<uint32_t, 4> isect_ti = { umax, umax, umax, 0 };
            sm::vec<float> isect_tn = { fmax, fmax, fmax };

            auto isect_d = std::numeric_limits<float>::max(); // distance to intersect

            const auto vdsos = vdir.sos();

            // Have we been passed a 'most likely triangle' to test first? If so, test it.
            if (ti_ml[0] != std::numeric_limits<uint32_t>::max()) {
                sm::vec<float> v0 = this->vertex[ti_ml[0]];
                sm::vec<float> v1 = this->vertex[ti_ml[1]];
                sm::vec<float> v2 = this->vertex[ti_ml[2]];
                auto [isect, p] = sm::geometry::ray_tri_intersection<float, float, true, false> (v0, v1, v2, vstart, vdir);
                if (isect) {
                    float d = (p - vstart).sos();
                    if (d < vdsos) {
                        sm::vec<sm::vec<float>, 3> tverts = { v0, v1, v2 };
                        isect_p = p;
                        isect_ti = ti_ml;
                        isect_tn = this->triangle_normal (tverts); // compute tn
                        isect_d = d;
                    }
                }
            }
            if (isect_d != std::numeric_limits<float>::max()) {
                // we found it already!
                return { isect_p, isect_ti, isect_tn };
            }

            for (auto tri : this->triangles) {
                auto [ti, tn, tnc, tnd] = tri;
                auto [isect, p] = sm::geometry::ray_tri_intersection<float, float, true, false> (this->vertex[ti[0]], this->vertex[ti[1]], this->vertex[ti[2]], vstart, vdir);
                // What if the triangle is one on the *other side of the model*?? Have to use
                // vdir.sos() to exclude those that are too far and the distance^2 to find the
                // closest one that isn't.
                if (isect) {
                    float d = (p - vstart).sos();
                    if (d < isect_d && d < vdsos) {
                        isect_p = p;
                        isect_ti = ti;
                        isect_tn = tn;
                        isect_d = d;
                    }
                }
            }

            if (isect_p[0] == fmax && this->vertex.size() < 10000) {
                // Found no triangle intersection; check vertices, in case vdir points perfectly at a vertex.
                // This can be computationally expensive, hence the hacky check, above.
                for (uint32_t ti = 0; ti < this->vertex.size(); ++ti) {
                    sm::vec<float> vertex_n = this->find_vertex_normal (ti); // also loops
                    vertex_n.renormalize();
                    vstart = coord_mf + (vertex_n / 2.0f);
                    if (sm::geometry::ray_point_intersection (this->vertex[ti], vstart, -vertex_n)) {
                        float d = (this->vertex[ti] - vstart).sos();
                        if (d < isect_d && d < vdir.sos()) {
                            std::cout << "Register vertex triangle_crossing\n";
                            isect_p = this->vertex[ti];
                            auto [_ti, _tn] = this->first_triangle_containing (ti);
                            isect_ti = _ti;
                            isect_tn = _tn;
                            isect_d = d;
                        }
                    }
                }
            }

            return { isect_p, isect_ti, isect_tn };
        }

        // Find the location, and the triangle indices at which a ray between coord (in model frame)
        // and the model centroid cross - the 'penetration point'.
        std::tuple<sm::vec<float>, std::array<uint32_t, 4>, sm::vec<float>>
        find_triangle_crossing (const sm::vec<float>& coord_mf) const
        {
            sm::vec<float> vdir = this->bb.mid() - coord_mf;
            vdir.renormalize();
            return this->find_triangle_crossing (coord_mf, vdir);
        }

        // Find a triangle containing indices a and b that isn't 'not_this' and return, along with its normal.
        std::tuple<std::array<uint32_t, 4>, sm::vec<float>>
        find_other_triangle_containing (const uint32_t a, const uint32_t b, const std::array<uint32_t, 4>& not_this) const
        {
            constexpr uint32_t umax = std::numeric_limits<uint32_t>::max();
            std::array<uint32_t, 4> other = {umax, umax, umax, 0};
            constexpr float fmax = std::numeric_limits<float>::max();
            sm::vec<float> other_n = {fmax, fmax, fmax};
            for (auto tri : triangles) {
                auto [ti, tn, tnc, tnd] = tri;
                if (ti[0] == not_this[0] && ti[1] == not_this[1] && ti[2] == not_this[2]) {
                    continue;
                }
                if ((ti[0] == a && (ti[1] == b || ti[2] == b))
                    || (ti[1] == a && (ti[0] == b || ti[2] == b))
                    || (ti[2] == a && (ti[0] == b || ti[1] == b))) {
                    other = ti;
                    other_n = tn;
                    break;
                }
            }
            return {other, other_n};
        }

        sm::vec<float> find_vertex_normal (const uint32_t ti) const
        {
            auto neighbs = this->find_neighbours (ti);
            sm::vec<float> vn = {};
            if (neighbs.size() == 0) { return vn; }
            for (auto nb : neighbs) {
                auto [ti, tn] = nb;
                vn += tn;
            }
            return (vn / neighbs.size());
        }

        // Find the common vertex (ignoring a/b[3]) between a and b
        uint32_t common_vertex (const std::array<uint32_t, 4>& a, const std::array<uint32_t, 4>& b)
        {
            uint32_t cv = std::numeric_limits<uint32_t>::max();
            if (a[0] == b[0] || a[1] == b[0] || a[2] == b[0]) {
                cv = b[0];
            } else if (a[0] == b[1] || a[1] == b[1] || a[2] == b[1]) {
                cv = b[1];
            } else if (a[0] == b[2] || a[1] == b[2] || a[2] == b[2]) {
                cv = b[2];
            }
            return cv;
        }

        // Flags class
        enum class pm_fl : uint32_t
        {
            no_cross_point, // Means 'there was no crossing'
            colinear        // Means the movement was colinear with an edge
        };
        /*
         * The partial movement that takes us to the crossing point, specified as movement + endpoint
         * (rather than startpoint + movement)
         */
        struct partial_movement
        {
            // The movement vector
            sm::vec<float> mv = {};
            // The end coordinate of the movement
            sm::vec<float> end = {};
            // boolean state
            sm::flags<pm_fl> flags;
        };

        /*
         * Find the part of mv_inplane that gets us to the triangle boundary defined by edge_s and
         * edge_e
         *
         * IS IS ASSUMED that mv_s is in the triangle plane and that a movement of mv_inplane would cross
         * the edge if it were long enough.
         *
         * All vectors and coordinates here are in the same coordinate frame as the triangle
         * vertices. That could be either the model frame OR the scene frame (but always one or the
         * other).
         *
         * \param edge_s Starting coordinate of the edge
         * \param edge_e End coordinate of the edge
         * \param t_norm The triangle normal vector
         * \param mv_s The movement starting point
         * \param mv_inplane The planned movement, starting from hovlocn
         *
         * \return a struct containing the partial movement vector and the end of the movement as a
         * coordinate. If mv_inplane does not cross the edge, then the return object contains the vector
         * mv_inplane itself, and the coordinate that this movement ends at.
         */
        partial_movement find_edge_crossing (const sm::vec<float>& edge_s,
                                             const sm::vec<float>& edge_e,
                                             const sm::vec<float>& t_norm,
                                             const sm::vec<float>& mv_s,
                                             const sm::vec<float>& mv_inplane)
        {
            constexpr bool debug = false;
            partial_movement pm;
            sm::vec<float> edge = edge_e - edge_s;

            sm::vec<float> u_y = edge;
            u_y.renormalize();
            sm::vec<float> u_z = t_norm;
            u_z.renormalize();
            sm::vec<float> u_x = u_y.cross (u_z);
            if constexpr (debug) {
                std::cout << "fec: mv_inplane = " << mv_inplane << std::endl;
                std::cout << "fec: edge = " << edge << std::endl;
                std::cout << "fec: Basis: " << u_x << " " << u_y << " " << u_z << std::endl;
            }

            // Create a matrix to convert from mdl frame movements to the triangle frame of ref.
            sm::mat<float, 4> from_triangle_frame = sm::mat<float, 4>::frombasis (u_x, u_y, u_z);
            sm::mat<float, 4> to_triangle_frame = from_triangle_frame.inverse();

            // Use Edge as our 'y' and the orthogonal as our 'x', then express mv_inplane in terms
            // of these two unit vectors. We also have our 'z' which is the triangle normal.
            sm::vec<float, 4> mv_inplane4d = to_triangle_frame * mv_inplane;
            sm::vec<float, 2> mv_inplane2d = { mv_inplane4d[0], mv_inplane4d[1] };
            sm::vec<float, 4> h_4d = to_triangle_frame * mv_s;
            sm::vec<float, 2> h_2d =  { h_4d[0], h_4d[1] };
            sm::vec<float, 4> edge_4d = to_triangle_frame * edge;
            sm::vec<float, 2> edge_2d =  { edge_4d[0], edge_4d[1] };
            sm::vec<float, 4> edge_s_4d = to_triangle_frame * edge_s;
            sm::vec<float, 2> edge_s_2d =  { edge_s_4d[0], edge_s_4d[1] };

            // Can now apply algo to find crossing point
            if constexpr (debug) {
                std::cout << "fec: intersection test for lines: " << edge_s_2d << " --> " << (edge_2d + edge_s_2d)
                          << " and " << h_2d << " --> " << (h_2d + mv_inplane2d) << "\n";
            }

            std::bitset<2> si = sm::geometry::segments_intersect<float> (edge_s_2d, edge_s_2d + edge_2d, h_2d, h_2d + mv_inplane2d);
            if (si.test(1)) {
                if constexpr (debug) { std::cout << "fec: Colinear with triangle edge!\n"; }
                pm.flags.set (pm_fl::colinear, true);
                // Identify the vertex that we're moving towards. edge_4d is the triangle edge.
                // so: mv_inplane4d.dot (edge_4d) should be positive if edge_e is the vertex
                sm::vec<float> mv_inplane3d = mv_inplane4d.less_one_dim();
                sm::vec<float> edge_e_3d = (to_triangle_frame * edge_e).less_one_dim();
                sm::vec<float> edge_s_3d = edge_s_4d.less_one_dim();

                if constexpr (debug) {
                    std::cout << "mv_inplane: " << mv_inplane3d << ", edge_e: " << edge_e_3d << ", edge_s: " << edge_s_3d << std::endl;
                    std::cout << "mv_inplane.dot (edge_e): " << mv_inplane3d.dot (edge_e_3d) << std::endl;
                    std::cout << "mv_inplane.dot (edge_s): " << mv_inplane3d.dot (edge_s_3d) << std::endl;
                }
                sm::vec<float> to_v = {};
                if (mv_inplane3d.dot (edge_e_3d) > mv_inplane3d.dot (edge_s_3d)) {
                    to_v = edge_e_3d - (h_4d).less_one_dim();
                } else {
                    to_v = edge_s_3d - (h_4d).less_one_dim();
                }

                if (to_v.length() <= mv_inplane3d.length()) {
                    if constexpr (debug) { std::cout << "fec: partial colinear move to vertex\n"; }
                    pm.flags.set (pm_fl::no_cross_point, false);
                    pm.mv = (from_triangle_frame * to_v).less_one_dim(); // need to know if we were to go over a vertex
                    pm.end = (from_triangle_frame * edge_e_3d).less_one_dim();
                } else {
                    if constexpr (debug) { std::cout << "fec: partial colinear along/within edge\n"; }
                    pm.flags.set (pm_fl::no_cross_point, true);
                    // Compute end from mv_inplane4d
                    pm.mv = (from_triangle_frame * mv_inplane4d).less_one_dim();
                    pm.end = (from_triangle_frame * (h_4d + mv_inplane4d)).less_one_dim();
                }

            } else {
                if (si.test(0)) {
                    // Intersects as expected
                    sm::vec<float, 2> cp2d = sm::geometry::crossing_point<float> (edge_s_2d, edge_s_2d + edge_2d, h_2d, h_2d + mv_inplane2d);
                    // Now go from cross point 2d to a point in model coordinates?
                    pm.end = (from_triangle_frame * cp2d.plus_one_dim(edge_s_4d[2])).less_one_dim();
                    if constexpr (debug) { std::cout << "fec: Cross point in mdl frame: " << pm.end << std::endl; }
                    pm.mv = pm.end - mv_s;

                } else {
                    // 'No intersection' can occur when: the movement goes over/close to the end of the edge.
                    // Or when: the move starts ON the edge of a triangle and then moves *away* from the tri.
                    if constexpr (debug) {
                        std::cout <<  "fec: No intersection across edge for: "
                                  << (edge_s_2d) << " -- " << (edge_2d + edge_s_2d) << " and "
                                  << h_2d << " -- " << (h_2d + mv_inplane2d) << std::endl;
                    }
                    // Mark that there was no intersection
                    pm.flags.set (pm_fl::no_cross_point, true);
                    pm.mv = sm::vec<float>{};
                    pm.end = mv_s;
                }
            }

            return pm;
        }

        /*
         * After testing up to all three edges of a triangle, we return information about the crossing
         * location and the indices of the triangle that form the crossed edge in this structure.
         */
        struct crossing_data
        {
            // edge_idx_a/b are the indices of the triangle vertices on the crossed edge
            uint32_t edge_idx_a = 0;
            uint32_t edge_idx_b = 0;
            // The crossed edge as a vector
            sm::vec<float> tri_edge = {};
            // The partial movement. pm.mv is the movement, pm.end is the crossing point
            partial_movement pm = {};
        };

        /*
         * Find the location at which a movement from mv_s in the direction mv_inplane crosses one of
         * the edges of the triangle specified by the three vertices in t_verts/t_indices.
         *
         * IT IS ASSUMED that the triangle normal passing through mv_s WILL intersect the
         * triangle (this may include an edge or vertex intersection). (Test beforehand with sm::geometry::ray_tri_intersection)
         *
         * All coordinates are in the frame of the model that contains this triangle.
         *
         * \param t_verts *Ordered* vertices of the triangle. Vertices should be in anti-clockwise order
         * when viewed with the triangle normal vector coming 'out of the page'
         *
         * \param t_indices The *Ordered* indices of the vertices in t_verts. Used to return the crossed
         * edge specified as two common indices. See t_verts for correct order of triangle vertices.
         *
         * \param mv_s The start of the planned movement
         *
         * \param mv_inplane The planned movement
         *
         * \param t_norm The triangle normal. While this could be computed from t_verts, it has already
         * been computed by the program and so I'm passing it in here.
         */
        crossing_data compute_crossing_location (const sm::vec<sm::vec<float>, 3>& t_verts,
                                                 const std::array<uint32_t, 4>& t_indices,
                                                 const sm::vec<float>& mv_s,
                                                 const sm::vec<float>& mv_inplane,
                                                 const sm::vec<float>& t_norm)
        {
            constexpr bool debug = false;
            crossing_data cd;
            cd.pm.flags.set (pm_fl::no_cross_point, true);

            const sm::vec<float>& t0 = t_verts[0];
            const sm::vec<float>& t1 = t_verts[1];
            const sm::vec<float>& t2 = t_verts[2];

            sm::vec<float> p = mv_s + mv_inplane;
            sm::vec<float> edge = t1 - t0;
            sm::vec<float> ptoe = p - t0;
            bool inside01 = (t_norm.dot (edge.cross (ptoe)) >= 0);
            if (!inside01) {
                partial_movement pm = find_edge_crossing (t0, t1, t_norm, mv_s, mv_inplane);
                if constexpr (debug) {
                    if (pm.flags.test (pm_fl::colinear)) {
                        std::cout << "ccl: fec returned pm.colinear true for t0t1\n";
                    }
                }
                if (pm.flags.test (pm_fl::no_cross_point)
                    && pm.flags.test (pm_fl::colinear) == false) {
                    inside01 = true;
                    if constexpr (debug) {
                        std::cout << "ccl: No intersection for edge t0t1 " << t0 << " -- " << t1
                                  << " and move " << mv_s << " -- " << (mv_s + mv_inplane) << std::endl;
                    }
                } else {
                    if constexpr (debug) {
                        if (pm.flags.test (pm_fl::colinear)) { std::cout << "ccl: colinear t0t1\n"; }
                        std::cout << "ccl: Intersection for edge t0t1 " <<  t0 << " -- " << t1
                                  << " and move " << mv_s << " -- " << (mv_s + mv_inplane) << std::endl;
                    }
                    cd.pm = pm;
                    cd.tri_edge = edge;
                    cd.edge_idx_a = t_indices[0];
                    cd.edge_idx_b = t_indices[1];
                }
            }

            edge = t2 - t1; ptoe = p - t1;
            bool inside21 = (t_norm.dot (edge.cross (ptoe)) >= 0);
            if (!inside21) {
                partial_movement pm = find_edge_crossing (t1, t2, t_norm, mv_s, mv_inplane);
                if constexpr (debug) {
                    if (pm.flags.test (pm_fl::colinear)) {
                        std::cout << "ccl: fec returned pm.colinear true for t1t2\n";
                    }
                }
                if (pm.flags.test (pm_fl::no_cross_point)
                    && pm.flags.test (pm_fl::colinear) == false) {
                    inside21 = true;
                    if constexpr (debug) {
                        std::cout << "ccl: No intersection for edge t1t2 " << t1 << " -- " << t2
                                  << " and move " << mv_s << " -- " << (mv_s + mv_inplane) << std::endl;
                    }
                } else {
                    if constexpr (debug) {
                        if (pm.flags.test (pm_fl::colinear)) { std::cout << "ccl: colinear t1t2\n"; }
                        std::cout << "ccl: Intersection for edge t1t2 " <<  t1 << " -- " << t2
                                  << " and move " << mv_s << " -- " << (mv_s + mv_inplane) << std::endl;
                    }
                    cd.pm = pm;
                    cd.tri_edge = edge;
                    cd.edge_idx_a = t_indices[2];
                    cd.edge_idx_b = t_indices[1];
                }
            }

            edge = t0 - t2; ptoe = p - t2;
            bool inside02 = (t_norm.dot (edge.cross (ptoe)) >= 0);
            if (!inside02) {
                partial_movement pm = find_edge_crossing (t2, t0, t_norm, mv_s, mv_inplane);
                if constexpr (debug) {
                    if (pm.flags.test (pm_fl::colinear)) {
                        std::cout << "ccl: fec returned pm.colinear true for t2t0\n";
                    }
                }
                if (pm.flags.test (pm_fl::no_cross_point)
                    && pm.flags.test (pm_fl::colinear) == false) {
                    inside02 = true;
                    if constexpr (debug) {
                        std::cout << "ccl: No intersection for edge t2t0 " << t2 << " -- " << t0
                                  << " and move " << mv_s << " -- " << (mv_s + mv_inplane) << std::endl;
                    }
                } else {
                    if constexpr (debug) {
                        if (pm.flags.test (pm_fl::colinear)) { std::cout << "ccl: colinear t2t0\n"; }
                        std::cout << "ccl: Intersection for edge t2t0 " <<  t2 << " -- " << t0
                                  << " and move " << mv_s << " -- " << (mv_s + mv_inplane) << std::endl;
                    }
                    cd.pm = pm;
                    cd.tri_edge = edge;
                    cd.edge_idx_a = t_indices[0];
                    cd.edge_idx_b = t_indices[2];
                }
            }

            // We've now tested edge crossing for three edges in the triangle.
            //
            if constexpr (debug) {
                if (cd.pm.flags.test (pm_fl::no_cross_point) == false) {
                    std::cout << "ccl: Crossed over" << (inside01 ? " " : " 0-1")
                              << (inside21 ? " " : " 2-1") <<  (inside02 ? " " : " 0-2") << std::endl;
                    // could test pairs of inside01 etc to detect crossing a vertex
                } else if (cd.pm.flags.test (pm_fl::colinear) == true) {
                    // Movement was colinear. Set Crossed vertex?
                    std::cout << "ccl: movement was colinear!\n";
                    if (cd.pm.flags.test (pm_fl::no_cross_point)) {
                        std::cout << "ccl: Colinear along edge" << std::endl;
                    } else {
                        std::cout << "ccl: Colinear to vertex" << std::endl;
                    }
                    // cd.pm.no_cross_point will tell if there's a cross point or not
                } else {
                    // We have NO crossing, which can occur for a variety of reasons
                    std::cout << "ccl: No crossings " << (inside01 ? " " : "!!0-1")
                              << (inside21 ? " " : "!!2-1") <<  (inside02 ? " " : "!!0-2") << std::endl;
                }
            }

            return cd;
        }

        /*!
         * Find the model location, starting from the location of a camera specified in
         * camspace. Cast a ray in the direction \a vdir, starting from the camera location in the
         * model frame \a camloc_mf, and figure out which triangle in the navmesh the ray passes
         * through.
         *
         * \param model_to_scene The model to scene transformation for the parent of the navmesh
         *
         * \param camloc_mf The camera location in the model frame. This gives us the start location
         * for the ray.
         *
         * \param vdir The direction of the ray.
         *
         * \param search_dist_mult a multiplier on the search distance. The length of vdir in this
         * function should cross the landscape model. By default it's the vector from the camera
         * location in the model frame of reference to the middle of the bounding box. If the vector
         * is too long when finding the surface of a convex hull, such as a model of a rock, it is
         * possible to mis-identify the back side of the model. However, for finding a location on a
         * large, flat, one-sided landscape, we want to make vdir long. search_dist_mult is applied
         * to vdir.
         *
         * \param ti_ml The most likely triangle, if you know what it probably is, to reduce the
         * search time.
         *
         * \return tuple containing: the hit point in scene coordinates; the triangle normal of the
         * triangle we hit; and the indices of the triangle we hit.
         */
        std::tuple<sm::vec<float>, sm::vec<float>, std::array<uint32_t, 4>>
        find_triangle_hit (const sm::mat<float, 4>& model_to_scene,
                           const sm::vec<float>& camloc_mf, const sm::vec<float>& vdir,
                           const std::array<uint32_t, 4> ti_ml = {std::numeric_limits<uint32_t>::max()})
        {
            this->ti0 = {};
            this->tn0 = {};
            sm::vec<float> hit = {};
            // Want to pass 'best tri' to this
            std::tie (hit, this->ti0, this->tn0) = this->find_triangle_crossing (camloc_mf, vdir, ti_ml);

            if (this->ti0[0] == std::numeric_limits<uint32_t>::max()) { std::cout << __func__ << ": No hit\n"; }

            sm::vec<float> hp_scene = (model_to_scene * hit).less_one_dim();

            constexpr bool debug = false;
            if constexpr (debug) {
                std::cout << "found hit at " << hit << " (model); " << hp_scene << " (scene) in direction " << vdir << "\n";
                // Check we'll get a hit when we compute_mesh_movement:
                sm::vec<sm::vec<float>, 3> tv_mf = this->triangle_vertices (this->ti0);
                std::cout << "tn0: " << this->tn0 << ", length " << this->tn0.length() << std::endl;
                std::cout << "TEST ray_tri_intersection (hit,-tn0): " << (hit + (this->tn0 / 2.0f)) << "," << -this->tn0 << std::endl;
                auto [isect, hov_mf] = sm::geometry::ray_tri_intersection<float> (tv_mf[0], tv_mf[1], tv_mf[2], hit + (this->tn0 / 2.0f), -this->tn0);
                if (isect) {
                    std::cout << "ray_tri_intersection confirms we would hit at " << hov_mf << "\n";
                } else {
                    std::cout << "ray_tri_intersection DOES NOT get a hit\n";
                    //throw std::runtime_error ("ray_tri_intersection DOES NOT get a hit!");
                }
            }

            return { hp_scene, this->tn0, this->ti0 };
        }

        /*!
         * Find the model location, starting from the location of a camera specified in
         * camspace. Cast a ray towards the centroid of this navmesh and figure out which triangle
         * in the navmesh the ray passes through.
         *
         * \param camspace The camera transformation matrix that converts camera coordinates into
         * the scene frame. This gives us the start location for the ray.
         *
         * \param model_to_scene The model to scene transformation for the parent of the navmesh
         *
         * \param search_dist_mult a multiplier on the search distance. The length of vdir in this
         * function should cross the landscape model. By default it's the vector from the camera
         * location in the model frame of reference to the middle of the bounding box. If the vector
         * is too long when finding the surface of a convex hull, such as a model of a rock, it is
         * possible to mis-identify the back side of the model. However, for finding a location on a
         * large, flat, one-sided landscape, we want to make vdir long. search_dist_mult is applied
         * to vdir.
         *
         * \return tuple containing: the hit point in scene coordinates; the triangle normal of the
         * triangle we hit; and the indices of the triangle we hit.
         */
        std::tuple<sm::vec<float>, sm::vec<float>, std::array<uint32_t, 4>>
        find_triangle_hit (const sm::mat<float, 4>& camspace, const sm::mat<float, 4>& model_to_scene,
                           const float search_dist_mult = 1.0f)
        {
            sm::mat<float, 4> scene_to_model = model_to_scene.inverse();
            // use camera location in gltf to start from, then find model surface.
            sm::vec<float> camloc_mf = (scene_to_model * camspace * sm::vec<float>{}).less_one_dim();
            sm::vec<float> vdir = this->bb.mid() - camloc_mf;
            vdir *= search_dist_mult;

            return this->find_triangle_hit (model_to_scene, camloc_mf, vdir);
        }

        /*!
         * Position the camera hoverheight above the location hp_scene, with its forward direction
         * _z and its 'x' axis in direction _x.
         */
        sm::mat<float, 4> position_camera (const sm::vec<float>& hp_scene, const sm::mat<float, 4>& model_to_scene,
                                           const sm::vec<float>& _x, const sm::vec<float>& _z,
                                           const float hoverheight)
        {
            // I think this positions correctly now (which is all it has to do). It ignores scaling
            // in model_to_scene. Can be reduced to use fewer mat<>s.
            sm::mat<float, 4> cam_mv_y;
            cam_mv_y.translate (sm::vec<float>{0, hoverheight, 0});
            // The basis _x, tn0, _z, where these are vectors in the model frame that define a camera frame
            sm::mat<float, 4> cam_to_model_rotn = sm::mat<float, 4>::frombasis (_x, this->tn0, _z);
            // Get the rotation from scene frame to model
            sm::mat<float, 4> m_to_sc_rotn = model_to_scene.rotation_mat44();
            sm::mat<float, 4> hp_m;
            hp_m.translate (hp_scene);
            sm::mat<float, 4> coord_rotn = hp_m * m_to_sc_rotn * cam_to_model_rotn * cam_mv_y;

            return coord_rotn;
        }

        /*!
         * Using data about the model location for the camera found with find_triangle_hit, return a
         * camera position matrix (scene frame)
         *
         * \return a transform matrix that places a camera frame of reference at hp_scene, oriented
         * with its y-axis in line with the normal of the triangle at the hit point, and with its x
         * and z axes randomly oriented. The frame is set to hover hoverheight 'above' the triangle
         */
        sm::mat<float, 4> position_camera (const sm::vec<float>& hp_scene, const sm::mat<float, 4>& model_to_scene,
                                           const float hoverheight)
        {
            // Let's 'draw' the camera towards the model and then arrange its normal upwards wrt to the normal of the model.
            if (this->tn0[0] == std::numeric_limits<float>::max()) {
                std::cout << __func__ << ": No hit/triangle normal\n";
                return sm::mat<float, 4>{};
            }

            // Place the camera on the model, and orient it randomly in the 'model plane'
            // The camera frame always has y up. Choose a random vector in the plane for 'x'
            // and then set z from this random x and the triangle norm (y).
            sm::vec<float> rand_vec;
            rand_vec.randomize();
            sm::vec<float> _x = rand_vec.cross (this->tn0);
            _x.renormalize();
            sm::vec<float> _z = _x.cross (this->tn0);

            return this->position_camera (hp_scene, model_to_scene, _x, _z, hoverheight);
        }

        /*!
         * A version of position camera that aligns the camera direction (i.e. where it is looking - its 'forwards')
         * as closely as possible with the passed-in vector
         */
        sm::mat<float, 4> position_camera (const sm::vec<float>& hp_scene, const sm::mat<float, 4>& model_to_scene,
                                           const float hoverheight, const sm::vec<float>& fwds)
        {
            // Let's 'draw' the camera towards the model and then arrange its normal upwards wrt to the normal of the model.
            if (this->tn0[0] == std::numeric_limits<float>::max()) {
                std::cout << __func__ << ": No hit/triangle normal\n";
                return sm::mat<float, 4>{};
            }

            // Project fwds onto the plane tn0
            sm::vec<float> _z = sm::geometry::vector_plane_projection (tn0, fwds);
            _z.renormalize();
            sm::vec<float> _x = -_z.cross (this->tn0);
            _x.renormalize();

            return this->position_camera (hp_scene, model_to_scene, _x, _z, hoverheight);
        }

        /*!
         * Compute a movement over this navigation mesh.
         *
         * We convert the triangle vertices from the model frame to the scene frame before computing
         * reorientations, so that non-uniform scalings in the model do not fox us.
         *
         * \param mv_camframe A movement vector in the camera's own frame of reference (an ego-motion)
         * \param cam_to_scene The transformation matrix to bring the camera coordinates to the scene frame
         * \param model_to_scene The transformation matrix to convert model coordinates to the scene frame
         * \param hoverheight
         *
         * \return The re-positioned camera transform matrix
         */
        sm::mat<float, 4> compute_mesh_movement (const sm::vec<float>& mv_camframe,
                                                 const sm::mat<float, 4>& cam_to_scene,
                                                 const sm::mat<float, 4>& model_to_scene,
                                                 const float hoverheight)
        {
            constexpr bool debug_move = false;
            constexpr bool debug_move2 = true;

            // A data-containing exception to throw
            mplot::NavException ne (mplot::NavException::type::generic);
            ne.tris.push_back (this->ti0);

            // Boolean state flags used in this function
            enum class cmm_fl : uint32_t { done, detected_crossing, single_movement, vertex_crossing };
            sm::flags<cmm_fl> flags;

            // Camera location, scene frame
            sm::vec<float> camloc_sf = cam_to_scene.translation();
            // Convert indices to vertices for triangle ti0, converting to the scene frame
            sm::vec<sm::vec<float>, 3> tv_sf = this->triangle_vertices (this->ti0, model_to_scene);
            // Compute the triangle normal in the scene frame
            this->tn0 = this->triangle_normal (tv_sf);

            if constexpr (debug_move) {
                std::cout << "\n# compute_mesh_movement:\n"
                          << "\nti0: " << this->ti0[0] << "," << this->ti0[1] << "," << this->ti0[2]
                          << "\nti0 (sf): " << tv_sf << "\nnormal " << this->tn0
                          << "\nmovement (camframe): " << mv_camframe
                          << "\nInitial camera location (camloc_sf): " << camloc_sf << "\n\n";
            }

            // Does camloc_sf in dirn tn0 intersect the tv_sf triangle? This
            // returns true if camloc_sf is on the edge of the triangle or on a
            // vertex. Assumes we're above the model and within the length of tn0 of the
            // surface.
            //
            // IF we're on an edge, then this intersection algo may disagree with
            // compute_crossing_location, which currently looks for crossing each of the three
            // boundaries and so requires that the start point is *within* the boundary.
            //
            if constexpr (debug_move) {
                std::cout << "First ray_tri_intersection (raystart,-tn0): " << (camloc_sf + (this->tn0 / 2.0f)) << "," << -this->tn0 << std::endl;
            }
            bool isect = false;
            sm::vec<float, 3> hov_sf = {};
            std::tie (isect, hov_sf) = sm::geometry::ray_tri_intersection<float> (tv_sf[0], tv_sf[1], tv_sf[2], camloc_sf + (this->tn0 / 2.0f), -this->tn0);

            // Use the detected location, hov_sf to compute the surface location of the camera - its 'hover location'
            sm::mat<float, 4> cam_to_surface = cam_to_scene;
            cam_to_surface.pretranslate (hov_sf - camloc_sf); // This is now our init pose; the camera is now at the surface

            // Try double precision
            if (isect == false) {
                std::tie (isect, hov_sf) = sm::geometry::ray_tri_intersection<float, double> (tv_sf[0], tv_sf[1], tv_sf[2], camloc_sf + (this->tn0 / 2.0f), -this->tn0);
                if constexpr (debug_move) {
                    if (isect == false) {
                        std::cout << "No isect at start with ti0 using float OR double internally" << std::endl;
                    } else {
                        std::cout << "Intersection at start with ti0 using *double* internally" << std::endl;
                    }
                }
            }

            // If that didn't work, try the triangle *vertices*
            uint32_t int_vertex = std::numeric_limits<uint32_t>::max(); // intersection vertex
            if (isect == false) {
                if constexpr (debug_move) { std::cout << "Try the triangle vertices...\n"; }
                for (uint32_t i = 0u; i < 3u; i++) {

                    // We need to use the *vertex* normal for this test - the average of all the adjacent triangle normals!
                    sm::vec<float> vertex_n = this->find_vertex_normal (this->ti0[i]);
                    vertex_n.renormalize();
                    if constexpr (debug_move) {
                        std::cout << "Vertex normal for triangle index " << ti0[i] << " is " << vertex_n << std::endl;
                    }

                    if (sm::geometry::ray_point_intersection (tv_sf[i], camloc_sf + (vertex_n / 2.0f), -vertex_n)) {
                        if constexpr (debug_move) {
                            std::cout << "A VERTEX intersection is the start at " << tv_sf[i] << ", compare this with hov_sf = " << hov_sf << "\n";
                            // if start is vertex, need to check movement across all the triangle-neighbours of this vertex (see later use of int_vertex)
                        }
                        hov_sf = tv_sf[i];
                        int_vertex = i;
                        isect = true;
                    }
                }
            }

            std::vector<std::array<uint32_t, 4>> trisearched; // the other triangles we search. To place in exception
            if (isect == false) {

                if constexpr (debug_move2) {
                    std::cout << "No intersection (at start) with triangle ti0, so correct ti0 and tn0 (if we can)" << std::endl;
                }

                // When very close to the boundary, ray_tri_intersection may fail. This triggers a
                // search for a neighbouring triangle which the camera may instead be hovering over
                // (this can occur when moving along an edge)
                for (uint32_t i = 0u; i < 3u; i++) {
                    uint32_t i1 = i;
                    uint32_t i2 = (i + 1) % 3u;
                    auto [_ti, _tn] = this->find_other_triangle_containing (this->ti0[i1], this->ti0[i2], this->ti0);
                    if (_ti[0] != std::numeric_limits<uint32_t>::max()) {
                        trisearched.push_back (_ti);
                        // Test to see if start location was inside a neighbour
                        sm::vec<sm::vec<float>, 3> tv_lf = this->triangle_vertices (_ti, model_to_scene);
                        // _tn was returned in model frame coordinates, so recompute in scene frame
                        _tn = this->triangle_normal (tv_lf);

                        auto [is, h] = sm::geometry::ray_tri_intersection<float> (tv_lf[0], tv_lf[1], tv_lf[2], camloc_sf + (_tn / 2.0f), -_tn);
                        if constexpr (debug_move) {
                            std::cout << "Start of move " << (is ? "IS" : "is NOT") << " in " << _ti[0] << "," << _ti[1] << "," << _ti[2] << std::endl;
                        }
                        if (is) {
                            if constexpr (debug_move) { std::cout << "CORRECT ti0 to " << _ti[0] << "," << _ti[1] << "," << _ti[2] << std::endl; }
                            // We're in this neighbour, so update ti0/tn0 and mark isect true
                            this->ti0 = _ti;
                            tv_sf = tv_lf;
                            this->tn0 = _tn;
                            isect = true;
                            // This requires a number of matrix recomputations:
                            hov_sf = h;
                            cam_to_surface = cam_to_scene;
                            cam_to_surface.pretranslate (hov_sf - camloc_sf); // This is our init pose, placed on the surface
                            break;
                        }
                    } // else missing neighbour. Could see if it would land in a neighbour that's just off the edge?
                }

                if (isect == false) {
                    if constexpr (debug_move2) {
                        std::cout << "DBG No intersection (at start) with triangle ti0 OR neighbours" << std::endl;
                    }

                    // Final test to see if we're on boundary?
                    float closest_edge_d = sm::geometry::dist_to_tri_edge (tv_sf[0], tv_sf[1], tv_sf[2], camloc_sf - (this->tn0 * hoverheight));
                    if constexpr (debug_move2) {
                        std::cout << "Closest distance from " << (camloc_sf - (this->tn0 * hoverheight))
                                  << " to ti0 edge: " << closest_edge_d << std::endl;
                    }
                    constexpr float ced_thresh = std::numeric_limits<float>::epsilon() * 50;
                    if (closest_edge_d < ced_thresh) {
                        // make tiny adjustment to camloc_sf so we ARE in the triangle? OR...
                        isect = true; // SAY we are, and proceed? <-- this if it works.
                    } else {
                        ne.m_type = NavException::type::no_intersection;
                        ne.tris.insert (ne.tris.end(), trisearched.begin(), trisearched.end());
                        throw ne;
                    }
                } else {
                    if constexpr (debug_move2) {
                        std::cout << "Found intersection (at start) with (2-)neighbour triangle "
                                  << this->ti0[0] << "," << this->ti0[1] << "," << this->ti0[2] << std::endl;
                    }
                }

            } else {
                if constexpr (debug_move) {
                    std::cout << "First ray_tri_intersected. Start of move is IN triangle ti0\n";
                }
            }

            // rest of function assumes isect was true (exception otherwise)

            // Find component of movement that is in the current triangle plane (in the scene frame of reference)
            sm::vec<float> mv_sf = (cam_to_scene * mv_camframe).less_one_dim() - camloc_sf;
            sm::vec<float> mv_orthog = this->tn0 * (mv_sf.dot (this->tn0) / (this->tn0.dot (this->tn0)));
            sm::vec<float> mv_inplane = mv_sf - mv_orthog; // scene frame, a relative movement

            if (mv_inplane.length() == 0.0f) {
                if constexpr (debug_move) { std::cout << "No movement, so return unchanged camera viewmatrix\n"; }
                return cam_to_scene;
            }

            // New section to handle the case that we started right on a vertex
            if (isect == true && int_vertex != std::numeric_limits<uint32_t>::max()) {
                // We HAVE a vertex intersection. Check if we either cross, or land in one of this vertex's neighbours to correct our starting triangle and normal.
                auto onens = this->find_neighbours (this->ti0[int_vertex]);
                for (auto onen : onens) {
                    auto [_ti, _tn] = onen;
                    sm::vec<float> _mv_orthog = _tn * (mv_sf.dot (_tn) / (_tn.dot (_tn)));
                    sm::vec<float> _mv_inplane = mv_sf - _mv_orthog; // scene frame, a relative movement
                    sm::vec<sm::vec<float>, 3> tv_nb = this->triangle_vertices (_ti, model_to_scene);
                    // _tn = this->triangle_normal (tv_nb); // shouldn't need to recompute
                    crossing_data cd = this->compute_crossing_location (tv_nb, _ti, hov_sf, _mv_inplane, _tn);
                    if (cd.pm.flags.test (pm_fl::no_cross_point) == false) {
                        this->ti0 = _ti;
                        this->tn0 = _tn;
                        tv_sf = tv_nb;
                        mv_orthog = _mv_orthog;
                        mv_inplane = _mv_inplane;
                        if constexpr (debug_move) {
                            std::cout << "Break on cross point with triangle (" << _ti[0] << "," << _ti[1] << "," << _ti[2] << ")\n";
                        }
                        break;
                    } else {
                        // No crossing, did we land in the triangle?
                        auto [is, h] = sm::geometry::ray_tri_intersection<float, double> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf + _mv_inplane + (_tn / 2.0f), -_tn);
                        if (is) { // then we DID land in this neighbour tri
                            this->ti0 = _ti;
                            this->tn0 = _tn;
                            tv_sf = tv_nb;
                            mv_orthog = _mv_orthog;
                            mv_inplane = _mv_inplane;
                            if constexpr (debug_move) {
                                std::cout << "Break as we landed in triangle (" << _ti[0] << "," << _ti[1] << "," << _ti[2] << ")\n";
                            }
                            break;
                        }
                    }
                }
            } // Now carry on with corrected mv_inplane, tn0 and ti0

            // A 'detected crossing' is one where we had to use a secondary method (comparing the
            // triangle containing the start and the triangle containing the end) to determine that
            // a triangle edge had been crossed, because the original method
            // (compute_crossing_location, which uses a faster, but numerically fallible approach)
            // failed.
            sm::vec<uint32_t, 2> detected_edge = {};
            sm::vec<float> detected_edgevec = {};
            std::array<uint32_t, 4> detected_newtri = {}; // new triangle detected as part of a vertex crossing

            // Now loop while our path may traverse one or more triangles
            while (!flags.test (cmm_fl::done)) {

                if constexpr (debug_move) {
                    std::cout << "\nWHILE LOOP\n"
                              << "ti0 = (" << this->ti0[0] << "," << this->ti0[1] << "," << this->ti0[2] << ")\n"
                              << "mv_inplane: " << hov_sf << "," << mv_inplane << "\n"
                              << "tn0 = " << this->tn0 << ")\n";
                }

                if (mv_inplane.length() == 0) {
                    ne.m_type = NavException::type::zero_mv;
                    throw ne;
                }
                if (mv_inplane.has_nan()) {
                    ne.m_type = NavException::type::nan_mv;
                    throw ne;
                }

                // Apply the edge crossing algorithm
                crossing_data cd = this->compute_crossing_location (tv_sf, this->ti0, hov_sf, mv_inplane, this->tn0);

                if (cd.pm.flags.test (pm_fl::no_cross_point) == false || flags.test (cmm_fl::detected_crossing) || flags.test (cmm_fl::vertex_crossing)) {
                    // Then an edge (or vertex)crossing WAS detected (by compute_crossing_location or a prev. 'detected crossing')

                    if (flags.test (cmm_fl::detected_crossing)) {
                        if constexpr (debug_move) {
                            std::cout << "This is a detected crossing; changing edge_idx_a/b to " << detected_edge << std::endl;
                        }
                        // We have to update our crossing data, as we detected a crossing over
                        // an edge (probably while moving along that edge)
                        cd.edge_idx_a = detected_edge[0];
                        cd.edge_idx_b = detected_edge[1];
                        cd.tri_edge = detected_edgevec;
                        cd.pm.mv = mv_inplane;
                        cd.pm.end = hov_sf + mv_inplane;
                    } else {
                        if constexpr (debug_move) {
                            std::cout << "This IS a crossing (compute_crossing_location found it) " << std::endl;
                        }
                    }

                    // _ti, _tn are the new triangle
                    sm::vec<float> _tn = {};
                    std::array<uint32_t, 4> _ti = {};
                    if (flags.test (cmm_fl::vertex_crossing)) {
                        if constexpr (debug_move) {
                            std::cout << "Setting _ti to over-the-vertex tri "
                                      << detected_newtri[0] << "-" << detected_newtri[0] << "-" << detected_newtri[0] << std::endl;
                        }
                        _ti = detected_newtri;
                    } else {
                        // Can work out new triangle here across the crossed edge
                        if constexpr (debug_move) {
                            std::cout << "find triangle across edge: find_other_triangle_containing ("
                                      << cd.edge_idx_a << ", " <<  cd.edge_idx_b
                                      << ", [" << this->ti0[0] << "," << this->ti0[1] << "," << this->ti0[2] << "])" << std::endl;
                        }
                        std::tie (_ti, _tn) = this->find_other_triangle_containing (cd.edge_idx_a, cd.edge_idx_b, this->ti0);
                    }

                    if (_ti[0] != std::numeric_limits<uint32_t>::max()) {

                        // Re-orient onto the new triangle
                        sm::vec<sm::vec<float>, 3> newtv_sf = this->triangle_vertices (_ti, model_to_scene);
                        _tn = this->triangle_normal (newtv_sf);

                        if constexpr (debug_move) {
                            std::cout << "RE-ORIENT to _ti: " << _ti[0] << "," << _ti[1] << "," << _ti[2]
                                      << "\n              " << newtv_sf << "\n              normal " << _tn << "\n";
                        }

                        // If a vertex crossing, we have to make an edge that is the cross product of the two triangle normals
                        if (flags.test (cmm_fl::vertex_crossing)) { cd.tri_edge = this->tn0.cross (_tn); }

                        // Compute the reorientation due to the requested movement.
                        float rotn_angle = 0.0f;
                        // Rotate by the angle between the normals (if stabilised is false). I think this is constrained to be <= pi
                        if (stabilised == false) { rotn_angle = this->tn0.angle (_tn, cd.tri_edge); }
                        // If tn0 and _tn are identical, then rotn_angle will be NaN, but in that case we want no rotation
                        if (std::isnan (rotn_angle)) { rotn_angle = 0.0f; }
                        sm::mat<float, 4> reorient_model; // reorientation transformation in sf
                        reorient_model.rotate (cd.tri_edge, rotn_angle);
                        sm::vec<float> mv_rest = (reorient_model * (mv_inplane - cd.pm.mv)).less_one_dim();
                        reorient_model.pretranslate (hov_sf + cd.pm.mv + mv_rest);
                        reorient_model.translate (-hov_sf); // r_t_to + r_t1 = -(hov_sf + cd.pm.mv) + cd.pm.mv = -hov_sf

                        if (mv_rest.length() == 0) {
                            // The first movement to edge completed the movement. We actually landed ON the edge.
                            cam_to_surface = reorient_model * cam_to_surface;
                            flags.set (cmm_fl::done, true);
                        } else {
                            // There's additional movement to complete.
                            if constexpr (debug_move) { std::cout << "mv_rest length is " << mv_rest.length() << std::endl; }

                            // At this point, can test to see if the end point of the movement
                            // lands in the adjacent triangle. If so, we're done, if not, time
                            // for another loop.
                            sm::vec<float> endmv = (reorient_model * cam_to_surface * sm::vec<float>{}).less_one_dim();
                            // Is endmv in newtv_sf/_ti?
                            auto [isect2, isectpoint2] = sm::geometry::ray_tri_intersection<float, float> (newtv_sf[0], newtv_sf[1], newtv_sf[2],
                                                                                                           endmv + (_tn / 2.0f), -_tn);
                            if constexpr (debug_move) {
                                std::cout << "endmv = " << endmv << " DOES" << (isect2 ? "" : " NOT") << " land in new triangle\n";
                            }
                            if (isect2) {
                                // We DID land in the neighbouring triangle. We are done.
                                cam_to_surface = reorient_model * cam_to_surface;
                                flags.set (cmm_fl::done, true);
                            } else {
                                if constexpr (debug_move) { std::cout << "did we sail past or land on the boundary or land in a 1-neighbour?\n"; }
                                // Incomplete; We've sailed past newtv_sf. Or perhaps landed on the boundary???
                                // We need to
                                // set an end-point that is on newtv_sf, update hov_sf,
                                // then recurse.  also recompute the movement encoded in
                                // reorient_model
                                reorient_model.pretranslate (-mv_rest);
                                cam_to_surface = reorient_model * cam_to_surface;
                                hov_sf = cd.pm.end; // crossing data planned movement end
                                // Also update planned move, which is now shorter and in a new direction
                                tv_sf = newtv_sf;
                                mv_inplane = mv_rest;
                            }
                        }

                        this->ti0 = _ti;
                        ne.tris.push_back (this->ti0);
                        this->tn0 = _tn;

                    } else {
                        // other triangle not found?! We probably went off the edge of our navigation model mesh
                        ne.m_type = NavException::type::off_edge;
                        throw ne;
                        continue;
                    }

                } else { // NO triangle edge crossing was detected with compute_crossing_location

                    // We had intersection in ti0, but no apparent crossing over its edges.
                    // We may have moved entirely within the starting triangle or colinear with an edge. Test for these cases.

                    // Check if it was a colinear movement
                    if (cd.pm.flags.test (pm_fl::colinear)) {
                        if (cd.pm.flags.test (pm_fl::no_cross_point) == true) {
                            flags.set (cmm_fl::single_movement, true);
                        } else { // We've moved to a vertex, should have captured this case
                            ne.m_type = NavException::type::mv_to_vertex;
                            throw ne;
                        }
                    } else {
                        // Test if it was movement-within; the simplest case
                        if constexpr (debug_move) {
                            std::cout << "No cross point and not colinear.\n  Testing if "
                                      << (hov_sf + mv_inplane + (this->tn0 / 2.0f)) << "," << -this->tn0
                                      << " intersects tv_sf (" << tv_sf << "\n";
                        }
                        auto [single_mv, he] = sm::geometry::ray_tri_intersection<float, float> (tv_sf[0], tv_sf[1], tv_sf[2], hov_sf + mv_inplane + (this->tn0 / 2.0f), -this->tn0);
                        flags.set (cmm_fl::single_movement, single_mv);
                    }

                    if (flags.test (cmm_fl::single_movement)) {
                        if constexpr (debug_move) { std::cout << "End of movement is *still* in ti0, so move mv_inplane/mv_camframe\n"; }
                        // Perform simplest movement, which is just to translate by mv_inplane
                        cam_to_surface.pretranslate (mv_inplane);
                        flags.set (cmm_fl::done, true);

                    } else {
                        if constexpr (debug_move) {
                            std::cout << "End of movement is NOT in ti0 " << this->ti0[0] << "," << this->ti0[1] << "," << this->ti0[2] << ". Look for start neighbours\n";
                        }

                        // Test 3 neighbours across the edges to find any for which the start location is also within-boundary
                        flags.set (cmm_fl::detected_crossing, false);
                        flags.set (cmm_fl::vertex_crossing, false);
                        std::array<uint32_t, 4> _ti_2n = { std::numeric_limits<uint32_t>::max() };
                        sm::vec<float>_tn_2n = {};
                        for (uint32_t i = 0u; i < 3u; i++) {
                            uint32_t i1 = i;
                            uint32_t i2 = (i + 1) % 3u;
                            auto [_ti, _tn] = this->find_other_triangle_containing (this->ti0[i1], this->ti0[i2], this->ti0);
                            if (_ti[0] != std::numeric_limits<uint32_t>::max()) {
                                // Test to see if start location was inside a neighbour
                                sm::vec<sm::vec<float>, 3> tv_nb = this->triangle_vertices (_ti, model_to_scene);
                                _tn = this->triangle_normal (tv_nb);

                                auto [is, h] = sm::geometry::ray_tri_intersection<float, double> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf + (_tn / 2.0f), -_tn);
                                sm::vec<float> mv_orthog_nb = _tn * (mv_inplane.dot (_tn) / (_tn.dot(_tn)));
                                sm::vec<float> mv_inplane_nb = mv_inplane - mv_orthog_nb;
                                if constexpr (debug_move) {
                                    std::cout << "endis? ray_tri_intersection with " << (hov_sf + mv_inplane_nb + (_tn / 2.0f)) << "," << -_tn << std::endl;
                                }
                                auto [endis, endh] = sm::geometry::ray_tri_intersection<float, double> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf + mv_inplane_nb + (_tn / 2.0f), -_tn);
                                if constexpr (debug_move) {
                                    std::cout << "Start of move " << (is ? "IS" : "is NOT")
                                              << " in " << _ti[0] << "," << _ti[1] << "," << _ti[2] << " / " <<  tv_nb << std::endl;
                                    std::cout << "End of move " << (endis ? "IS" : "is NOT")
                                              << " in that triangle" << std::endl;
                                }

                                // Here, start is in original, end may not be in original. This
                                // is an 'intersection detected crossing' of a triangle edge
                                // which wasn't picked up with compute_crossing_location
                                if (endis) {
                                    // End is in neighbour so this is a detected crossing
                                    if constexpr (debug_move) { std::cout << "DETECTED crossing! Pass on to next loop!\n"; }
                                    flags.set (cmm_fl::detected_crossing, true);
                                    detected_edge = { this->ti0[i1], this->ti0[i2] };
                                    detected_edgevec = tv_nb[i2] - tv_nb[i1];
                                    break; // out of for
                                } else { // end not in neighbour
                                    if (is) { // start is in neighbour tri (will re-orient to this and re-loop)
                                        _ti_2n = _ti;
                                        _tn_2n = _tn;
                                        break; // out of for
                                    } // else end is not in neighbour, and neither is start. This
                                      // occurs if the end is ON the boundary, but precision errors
                                      // mean this location isn't 'in' either start or neighbour
                                      // (according to ray_tri_intersection)
                                }
                            }
                        }

                        // Test one-neighbours here if necessary (that is, if the two neighbour test above failed)
                        if (flags.test (cmm_fl::detected_crossing) == false &&
                            _ti_2n[0] == std::numeric_limits<uint32_t>::max()) {
                            auto onens = this->find_one_neighbours (this->ti0);
                            for (auto onen : onens) {
                                // Are we in this one?
                                auto [_ti, _tn] = onen;
                                sm::vec<sm::vec<float>, 3> tv_nb = this->triangle_vertices (_ti, model_to_scene);
                                _tn = this->triangle_normal (tv_nb);
                                auto [is, h] = sm::geometry::ray_tri_intersection<float, double> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf + (_tn / 2.0f), -_tn);
                                sm::vec<float> mv_orthog_nb = _tn * (mv_inplane.dot (_tn) / (_tn.dot(_tn)));
                                sm::vec<float> mv_inplane_nb = mv_inplane - mv_orthog_nb;
                                if constexpr (debug_move) {
                                    std::cout << "endis ONE-n? ray_tri_intersection with " << (hov_sf + mv_inplane_nb + (_tn / 2.0f)) << "," << -_tn << std::endl;
                                }
                                auto [endis, endh] = sm::geometry::ray_tri_intersection<float, double> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf + mv_inplane_nb + (_tn / 2.0f), -_tn);
                                if constexpr (debug_move) {
                                    std::cout << "Start of move " << (is ? "IS" : "is NOT")
                                              << " in ONE-neighbour " << _ti[0] << "," << _ti[1] << "," << _ti[2] << " / " <<  tv_nb << std::endl;
                                    std::cout << "And End of move " << (endis ? "IS" : "is NOT")
                                              << " in that ONE-neighbour " << std::endl;
                                }

                                if (endis) {
                                    // End is in one-neighbour so this is a detected crossing
                                    if constexpr (debug_move) { std::cout << "DETECTED crossing over ONE-neighbour! Pass on to next loop!\n"; }
                                    flags.set (cmm_fl::vertex_crossing, true);
                                    detected_edge = { this->common_vertex (this->ti0, _ti), std::numeric_limits<uint32_t>::max() };
                                    detected_edgevec = {}; // to be the cross product of the last-triangle normal and the newtri normal.
                                    detected_newtri = _ti;
                                    break; // out of for
                                } else { // end not in one-neighbour
                                    if (is) { // start is in one-neighbour tri (will re-orient to this and re-loop)
                                        _ti_2n = _ti;
                                        _tn_2n = _tn;
                                        break; // out of for
                                    } // else end is not in one-neighbour, and neither is start.
                                }
                            }
                        }

                        if (_ti_2n[0] != std::numeric_limits<uint32_t>::max()) {
                            // Now we know an alternative start triangle for the movement. Re-orient to this and re-loop
                            this->ti0 = _ti_2n;
                            ne.tris.push_back (this->ti0);
                            this->tn0 = _tn_2n;
                            // recompute mv_inplane for this neighbour triangle
                            mv_orthog = this->tn0 * (mv_sf.dot (this->tn0) / (this->tn0.dot (this->tn0)));
                            mv_inplane = mv_sf - mv_orthog; // sf
                        } else if (flags.test (cmm_fl::detected_crossing)) {
                            // We didn't find an alternative start triangle, but we did detect an edge crossing by intersection, so continue.
                        } else if (flags.test (cmm_fl::vertex_crossing)) {
                            // We didn't find an alternative start triangle, but we did detect a vertex crossing, so continue.
                        } else {
                            // End of move not evidently in self or neighbours, so assume it's bang on the boundary
                            if constexpr (debug_move2) { std::cout << "Movement complete on boundary ASSUMPTION\n"; }
                            cam_to_surface.pretranslate (mv_inplane);
                            flags.set (cmm_fl::done, true);
                        }

                    } // single movement if/else

                } // compute_crossing_location if/else

            } // triangle traversing while loop

            // Raise cam_to_surface up by hoverheight and then return
            cam_to_surface.pretranslate (hoverheight * this->tn0);
            if constexpr (debug_move) {
                std::cout << "looping mv_inplanes completed. Final camloc_sf: " << cam_to_surface.translation() << std::endl;
            }
            return cam_to_surface;

        } // compute_mesh_movement

    }; // struct NavMesh

} // namespace
