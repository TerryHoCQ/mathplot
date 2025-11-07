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

#include <sm/vec>
#include <sm/vvec>
#include <sm/flags>
#include <sm/mat44>

namespace mplot
{
    // Exception that returns triangles that were near the location of the error
    struct NavException : public std::exception
    {
        enum class type : uint32_t { generic, no_intersection, zero_mv, mv_to_vertex, undetected_crossing };

        NavException (const type _type) : m_type(_type) {}
        NavException (const type _type, const std::vector<std::array<uint32_t, 4>>& t) : m_type(_type) { this->tris = t; }

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
            case type::generic:
            default:
                break;
            }
            return "Generic";
        }
        // Error type determines message generated
        type m_type = type::generic;
        // Triangles of interest.
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
         * Maps index in vertex to the original parent->indices index. populated by
         * VisualModel::make_navmesh()
         */
        sm::vvec<sm::vvec<uint32_t>> vertexidx_to_indices;

        //! Holds a copy of the bb of the parent model
        sm::range<sm::vec<float>> bb;

        /*!
         * Return index of this->vertex that is closest to scene_coord. Can use vertexidx_to_indices
         * to find the indices into vertexPositions and vertexNormals that this index in the
         * topographic mesh relates to.
         *
         * \param scene_coord Supplied coordinate in scene frame of referencea
         * \param viewmatrix The viewmatrix of the model which converts model frame coordinates to the scene frame
         */
        uint32_t find_vertex_nearest (const sm::vec<float>& scene_coord, const sm::mat44<float>& viewmatrix) const
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
        sm::vec<sm::vec<float>, 3> triangle_vertices (const std::array<uint32_t, 4>& tri_indices, const sm::mat44<float>& transform) const
        {
            sm::vec<sm::vec<float>, 3> trivert;
            if (tri_indices[0] < this->vertex.size()) { trivert[0] = (transform * this->vertex[tri_indices[0]]).less_one_dim(); }
            if (tri_indices[1] < this->vertex.size()) { trivert[1] = (transform * this->vertex[tri_indices[1]]).less_one_dim(); }
            if (tri_indices[2] < this->vertex.size()) { trivert[2] = (transform * this->vertex[tri_indices[2]]).less_one_dim(); }
            return trivert;
        }

        // Compute the triangle normal for the ordered triplet of triangle vertices, tverts
        sm::vec<float, 3> triangle_normal (const sm::vec<sm::vec<float>, 3>& tverts)
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

        // Determine if ti0 is on the edge of the model (with < 3 edge neighbours), If so, place 1
        // in its final element.
        void mark_if_on_edge (std::array<uint32_t, 4>& ti0)
        {
            uint32_t n2 = 0; // Neighbours sharing 2 vertices (up to 3)
            for (auto t: this->triangles) {
                auto [ti, tn, tnc, tnd] = t;
                auto a0 = ti0[0];
                auto b0 = ti0[1];
                auto c0 = ti0[2];
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
            }

            if (n2 < 3) {
                std::cout << ti0[0] << "-" << ti0[1] << "-" << ti0[2] << " is on the edge\n";
                ti0[3] = 1;
            } // Meaning that the triangle is 'on the edge' of the model
        }

        // Go through all triangles, marking if they're an 'edge' triangle. A triangle is ALSO on
        // the edge if on of its neighbours has < 3 edge neighbours.
        void mark_edge_triangles()
        {
            uint32_t ec = 0;
            for (auto& t: this->triangles) {
                auto& [ti, tn, tnc, tnd] = t;
                mark_if_on_edge (ti);
                if (ti[3]) {

                    ec++;

                    // Mark its neighbours too. This'll slow things down
                    for (auto& t2: this->triangles) {
                        auto& [ti2, tn2, tnc2, tnd2] = t2;
                        if (ti2 == ti) { continue; }
                        // If ti2 shares 2 vertices with ti, then mark ti2 as 'edge' also
                        int ns = 0;
                        if (ti2[0] == ti[0] || ti2[1] == ti[0] || ti2[2] == ti[0]) { ++ns; }
                        if ( ti2[0] == ti[1] || ti2[1] == ti[1] || ti2[2] == ti[1]) { ++ns; }
                        if ( ti2[0] == ti[2] || ti2[1] == ti[2] || ti2[2] == ti[2]) { ++ns; }
                        if (ns == 2) {
                            std::cout << ti2[0]<<","<<ti2[1]<<","<<ti2[2] << " shares 2 vertices with "
                                      << ti[0]<<","<<ti[1]<<","<<ti[2] << ".\n";
                            ti2[3] = 1;
                            ec++;
                        }
                    }
                }
            }
            std::cout << ec << " / " << this->triangles.size() << " triangles are on edge\n";
        }

        // Count 2-vertex (i.e. edge) neighbours and also 1-vertex neighbours for triangle ti0
        std::tuple<uint32_t, uint32_t> count_neighbour_triangles (const std::array<uint32_t, 4>& ti0) const
        {
            // Count neighbour triangles
            uint32_t n1 = 0; // Neighbour sharing 1 vertex (any number)
            uint32_t n2 = 0; // Neighbours sharing 2 vertices (up to 3)
            for (auto t: this->triangles) {
                auto [ti, tn, tnc, tnd] = t;
                auto a0 = ti0[0];
                auto b0 = ti0[1];
                auto c0 = ti0[2];
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

        /*
         * Find the location, and the triangle indices at which a ray starting from coord (scene
         * frame) with direction vdir - the 'penetration point'.
         *
         * \return a tuple containing crossing location, triangle identity (three indices) and triangle normal vector
         */
        std::tuple<sm::vec<float>, std::array<uint32_t, 4>, sm::vec<float>>
        find_triangle_crossing (const sm::vec<float>& coord_mf, const sm::vec<float>& vdir) const
        {
            for (auto tri : triangles) {
                auto [ti, tn, tnc, tnd] = tri;
                auto [isect, p] = sm::algo::ray_tri_intersection<float, true, false> (this->vertex[ti[0]], this->vertex[ti[1]], this->vertex[ti[2]], coord_mf - (vdir / 2.0f), vdir);
                if (isect) { return {p, ti, tn}; }
            }

            // Failed to find, return container full of maxes
            sm::vec<float> p = {};
            p.set_from (std::numeric_limits<float>::max());
            constexpr uint32_t umax = std::numeric_limits<uint32_t>::max();
            return {p , std::array<uint32_t, 4>{umax, umax, umax, 0}, p};
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
            sm::mat44<float> from_triangle_frame = sm::mat44<float>::frombasis (u_x, u_y, u_z);
            sm::mat44<float> to_triangle_frame = from_triangle_frame.inverse();

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

            std::bitset<2> si = sm::algo::segments_intersect<float> (edge_s_2d, edge_s_2d + edge_2d, h_2d, h_2d + mv_inplane2d);
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
                    sm::vec<float, 2> cp2d = sm::algo::crossing_point<float> (edge_s_2d, edge_s_2d + edge_2d, h_2d, h_2d + mv_inplane2d);
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
         * triangle (this may include an edge or vertex intersection). (Test beforehand with sm::algo::ray_tri_intersection)
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
         * camspace. Cast a ray towards the centroid of this navmesh and figure out which triangle
         * in the navmesh the ray passes through.
         *
         * \param camspace The camera transformation matrix that converts camera coordinates into
         * the scene frame. This gives us the start location for the ray.
         *
         * \param model_to_scene The model to scene transformation for the parent of the navmesh
         *
         * \return tuple containing: the hit point in scene coordinates; the triangle normal of the
         * triangle we hit; and the indices of the triangle we hit.
         */
        std::tuple<sm::vec<float>, sm::vec<float>, std::array<uint32_t, 4>>
        find_triangle_hit (const sm::mat44<float>& camspace, const sm::mat44<float>& model_to_scene)
        {
            constexpr bool debug = true;
            sm::mat44<float> scene_to_model = model_to_scene.inverse();
            // use camera location in gltf to start from, then find model surface.
            sm::vec<float> camloc_mf = (scene_to_model * camspace * sm::vec<float>{}).less_one_dim();
            std::array<uint32_t, 4> ti0;
            sm::vec<float> tn0 = {};
            sm::vec<float> hit = {};
            sm::vec<float> vdir = this->bb.mid() - camloc_mf;
            float bb_len = this->bb.span().longest(); // lengthscale of model
            // Make vdir long
            float vdl = vdir.length() * 2.0f; // Twice the distance from camera to BB centroid
            vdl += bb_len * 2.0f;             // plus twice the longest axis from the BB
            vdir.renormalize();
            vdir *= vdl;
            std::tie (hit, ti0, tn0) = this->find_triangle_crossing (camloc_mf - (vdir / 2.0f), vdir);
            if (ti0[0] == std::numeric_limits<uint32_t>::max()) {
                std::cout << __func__ << ": No hit\n";
            }
            // Can I make hit the centre of the triangle?
            constexpr bool hit_tri_centre = false;
            if constexpr (hit_tri_centre) {
                sm::vec<sm::vec<float>, 3> tv_mf = this->triangle_vertices (ti0);
                hit = tv_mf.mean();
            }
            sm::vec<float> hp_scene = (model_to_scene * hit).less_one_dim();

            if constexpr (debug) {
                std::cout << "found hit at " << hit << " (model); " << hp_scene << " (scene)\n";
                // Check we'll get a hit when we compute_mesh_movement:
                sm::vec<sm::vec<float>, 3> tv_mf = this->triangle_vertices (ti0);
                std::cout << "tn0: " << tn0 << ", length " << tn0.length() << std::endl;
                std::cout << "TEST ray_tri_intersection (hit,-tn0): " << (hit + (tn0 / 2.0f)) << "," << -tn0 << std::endl;
                auto [isect, hov_mf] = sm::algo::ray_tri_intersection<float> (tv_mf[0], tv_mf[1], tv_mf[2], hit + (tn0 / 2.0f), -tn0);
                if (isect) {
                    std::cout << "ray_tri_intersection confirms we would hit at " << hov_mf << "\n";
                } else {
                    std::cout << "ray_tri_intersection DOES NOT get a hit\n";
                    //throw std::runtime_error ("ray_tri_intersection DOES NOT get a hit!");
                }
            }

            return { hp_scene, tn0, ti0 };
        }

        /*!
         * Using data about the model location for the camera found with find_triangle_hit, return a
         * camera position matrix (scene frame)
         *
         * \return a transform matrix that places a camera frame of reference at hp_scene, oriented
         * with its y-axis in line with the normal of the triangle at the hit point, and with its x
         * and z axes randomly oriented. The frame is set to hover hoverheight 'above' the triangle
         */
        sm::mat44<float> position_camera (const sm::vec<float>& hp_scene, const sm::mat44<float>& model_to_scene,
                                          const sm::vec<float>& tn0, const float hoverheight)
        {
            // Let's 'draw' the camera towards the model and then arrange its normal upwards wrt to the normal of the model.
            if (tn0[0] == std::numeric_limits<float>::max()) {
                std::cout << __func__ << ": No hit\n";
                return sm::mat44<float>{};
            }

            // Place the camera on the model, and orient it randomly in the 'model plane'
            // The camera frame always has y up. Choose a random vector in the plane for 'x'
            // and then set z from this random x and the triangle norm (y).
            sm::vec<float> rand_vec;
            rand_vec.randomize();
            sm::vec<float> _x = rand_vec.cross (tn0);
            _x.renormalize();
            sm::vec<float> _z = _x.cross (tn0);
#if 0
            // This was DEBUG code to get one kind of camera oriented exactly on an edge
            sm::vec<float> _x = {0,1,-1};
            _x.renormalize();
            sm::vec<float> _z = {0,-1,-1};
            _z.renormalize();
#endif

            // I think this positions correctly now (which is all it has to do). It ignores scaling
            // in model_to_scene. Can be reduced to use fewer mat44s.
            sm::mat44<float> cam_mv_y;
            cam_mv_y.translate (sm::vec<float>{0, hoverheight, 0});
            // The basis _x, tn0, _z, where these are vectors in the model frame that define a camera frame
            sm::mat44<float> cam_to_model_rotn = sm::mat44<float>::frombasis (_x, tn0, _z);
            // Get the rotation from scene frame to model
            sm::mat44<float> m_to_sc_rotn = model_to_scene.rotation_mat44();
            sm::mat44<float> hp_m;
            hp_m.translate (hp_scene);
            sm::mat44<float> coord_rotn = hp_m * m_to_sc_rotn * cam_to_model_rotn * cam_mv_y;

            return coord_rotn;
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
         * \param ti0 Triangle indices. Will be updated if movement passed to another triangle
         * \param hoverheight
         *
         * \return The re-positioned camera transform matrix
         */
        sm::mat44<float> compute_mesh_movement (const sm::vec<float>& mv_camframe,
                                                const sm::mat44<float>& cam_to_scene,
                                                const sm::mat44<float>& model_to_scene,
                                                std::array<uint32_t, 4>& ti0,
                                                const float hoverheight)
        {
            constexpr bool debug_move = true;

            // A data-containing exception to throw
            mplot::NavException ne (mplot::NavException::type::generic);
            ne.tris.push_back (ti0);

            // Boolean state flags used in this function
            enum class cmm_fl : uint32_t { done, detected_crossing, single_movement };
            sm::flags<cmm_fl> flags;

            // Camera location, scene frame
            sm::vec<float> camloc_sf = cam_to_scene.translation();
            // Convert indices to vertices for triangle ti0, converting to the scene frame
            sm::vec<sm::vec<float>, 3> tv_sf = this->triangle_vertices (ti0, model_to_scene);
            // Compute the triangle normal in the scene frame
            sm::vec<float> tn0 = this->triangle_normal (tv_sf);

            if constexpr (debug_move) {
                std::cout << "\n# compute_mesh_movement: ti0 " << ti0[0] << "," << ti0[1] << "," << ti0[2]
                          << " has vertices (sf) at " << tv_sf << " and normal " << tn0
                          << ". upcoming movement (camframe) is " << mv_camframe << std::endl;
                std::cout << "Initial camera location (camloc_sf): " << camloc_sf << std::endl;
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
            std::cout << "First ray_tri_intersection (raystart,-tn0): " << (camloc_sf + (tn0 / 2.0f)) << "," << -tn0 << std::endl;
            auto [isect, hov_sf] = sm::algo::ray_tri_intersection<float> (tv_sf[0], tv_sf[1], tv_sf[2], camloc_sf + (tn0 / 2.0f), -tn0);

            // Use the detected location, hov_sf to compute the surface location of the camera - its 'hover location'
            sm::mat44<float> cam_to_surface = cam_to_scene;
            cam_to_surface.pretranslate (hov_sf - camloc_sf); // This is now our init pose; the camera is now at the surface

            std::vector<std::array<uint32_t, 4>> trisearched; // the other triangles we search. To place in exception
            if (isect == false) {

                if constexpr (debug_move) {
                    std::cout << "No intersection (at start) with triangle "
                              << ti0[0] << "," << ti0[1] << "," << ti0[2]
                              << ", so correct ti0 and tn0 (if we can)" << std::endl;
                }

                // When very close to the boundary, ray_tri_intersection may fail. This triggers a
                // search for a neighbouring triangle which the camera may instead be hovering over
                // (this can occur when moving along an edge)
                for (uint32_t i = 0u; i < 3u; i++) {
                    uint32_t i1 = i;
                    uint32_t i2 = (i + 1) % 3u;
                    auto [_ti, _tn] = this->find_other_triangle_containing (ti0[i1], ti0[i2], ti0);
                    if (_ti[0] != std::numeric_limits<uint32_t>::max()) {
                        trisearched.push_back (_ti);
                        // Test to see if start location was inside a neighbour
                        sm::vec<sm::vec<float>, 3> tv_lf = this->triangle_vertices (_ti, model_to_scene);
                        // _tn was returned in model frame coordinates, so recompute in scene frame
                        _tn = this->triangle_normal (tv_lf);

                        auto [is, h] = sm::algo::ray_tri_intersection<float> (tv_lf[0], tv_lf[1], tv_lf[2], camloc_sf + (_tn / 2.0f), -_tn);
                        if constexpr (debug_move) {
                            std::cout << "Start of move " << (is ? "IS" : "is NOT") << " in " << _ti[0] << "," << _ti[1] << "," << _ti[2] << std::endl;
                        }
                        if (is) {
                            if constexpr (debug_move) { std::cout << "*** Correcting!\n"; }
                            // We're in this neighbour, so update ti0/tn0 and mark isect true
                            ti0 = _ti;
                            tn0 = _tn;
                            isect = true;
                            // This requires a number of matrix recomputations:
                            hov_sf = h;
                            cam_to_surface = cam_to_scene;
                            cam_to_surface.pretranslate (hov_sf - camloc_sf); // This is our init pose, placed on the surface
                            break;
                        }
                    } // else missing neighbour. Could see if it would land in a neighbour that's just off the edge?
                }
                // FIXME: Another way ray_tri_intersection may have failed is if the normal goes
                // through/very close to a vertex (of the original).
                if (isect == false) {
                    // This can occur when the agent is on the edge of an edge triangle.
                    if (trisearched.size() < 4) {
                        std::cout << "\nsearched 2 or fewer neighbour triangles\n";
                    }

                    std::cout << "\nCompare current location " << cam_to_scene.translation()
                              << " with ti0: " << this->triangle_vertices (ti0, model_to_scene) << "\n\n";

                    ne.m_type = NavException::type::no_intersection;
                    ne.tris.insert (ne.tris.end(), trisearched.begin(), trisearched.end());
                    throw ne;
                    // Rather than throw exception, would it be better to return an unchanged viewmatrix?
                    // Problem with this: is that we end up spinning and stuck on the edge
                }

            } else {
                if constexpr (debug_move) {
                    std::cout << "First ray_tri_intersected. Start of move is IN triangle "
                              << ti0[0] << "," << ti0[1] << "," << ti0[2]
                              << " from coord " << camloc_sf << " and dirn " << -tn0 << std::endl;
                }
            }

            // rest of function assumes isect was true (exception otherwise)

            // Find component of movement that is in the current triangle plane (in the scene frame of reference)
            sm::vec<float> mv_sf = (cam_to_scene * mv_camframe).less_one_dim() - camloc_sf;
            sm::vec<float> mv_orthog = tn0 * (mv_sf.dot (tn0) / (tn0.dot (tn0)));
            sm::vec<float> mv_inplane = mv_sf - mv_orthog; // scene frame, a relative movement

            if (mv_inplane.length() == 0.0f) {
                if constexpr (debug_move) { std::cout << "No movement, so return unchanged camera viewmatrix\n"; }
                return cam_to_scene;
            }

            // A 'detected crossing' is one where we had to use a secondary method (comparing the
            // triangle containing the start and the triangle containing the end) to determine that
            // a triangle edge had been crossed, because the original method
            // (compute_crossing_location, which uses a faster, but numerically fallible approach)
            // failed.
            sm::vec<uint32_t, 2> detected_edge = {};
            sm::vec<float> detected_edgevec = {};

            // Now loop while our path may traverse one or more triangles
            while (!flags.test (cmm_fl::done)) {

                if constexpr (debug_move) {
                    std::cout << "\n* loopstart: Processing mv_inplane: " << mv_inplane
                              << " from surface start " << hov_sf << std::endl;
                }

                if (mv_inplane.length() == 0) {
                    ne.m_type = NavException::type::zero_mv;
                    throw ne;
                }

                // For each edge in triangle, compute distance to edge for hov_sf and (hov_sf + mv_inplane)
                crossing_data cd = this->compute_crossing_location (tv_sf, ti0, hov_sf, mv_inplane, tn0);

                if (cd.pm.flags.test (pm_fl::no_cross_point) == false || flags.test (cmm_fl::detected_crossing)) {
                    // Then an edge crossing WAS detected (by compute_crossing_location or a prev. 'detected crossing')

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
                    }

                    // Can work out new triangle here
                    if constexpr (debug_move) {
                        std::cout << "find_other_triangle_containing ("
                                  << cd.edge_idx_a << ", " <<  cd.edge_idx_b
                                  << ", [" <<  ti0[0] << "," << ti0[1] << "," << ti0[2] << "])" << std::endl;
                    }

                    auto [_ti, _tn] = this->find_other_triangle_containing (cd.edge_idx_a, cd.edge_idx_b, ti0);

                    if (_ti[0] != std::numeric_limits<uint32_t>::max()) {

                        // Re-orient onto the new triangle
                        sm::vec<sm::vec<float>, 3> newtv_sf = this->triangle_vertices (_ti, model_to_scene);
                        _tn = this->triangle_normal (newtv_sf);

                        if constexpr (debug_move) {
                            std::cout << "Re-orient to new triangle " << _ti[0] << "," << _ti[1] << "," << _ti[2]
                                      << "[ " << newtv_sf << " ] with normal " << _tn << "\n";
                        }

                        // Compute the reorientation due to the requested movement.
                        // Rotate by the angle between the normals. I think this is constrained to be <= pi
                        float rotn_angle = tn0.angle (_tn, cd.tri_edge);
                        // If tn0 and _tn are identical, then rotn_angle will be NaN, but in that case we want no rotation
                        if (std::isnan (rotn_angle)) { rotn_angle = 0.0f; }
                        sm::mat44<float> reorient_model; // reorientation transformation in sf
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
                            // At this point, can test to see if the end point of the movement
                            // lands in the adjacent triangle. If so, we're done, if not, time
                            // for another loop.
                            sm::vec<float> endmv = (reorient_model * cam_to_surface * sm::vec<float>{}).less_one_dim();
                            // Is endmv in newtv_sf/_ti?
                            auto [isect2, isectpoint2] = sm::algo::ray_tri_intersection<float> (newtv_sf[0], newtv_sf[1], newtv_sf[2],
                                                                                                endmv + (_tn / 2.0f), -_tn);
                            if constexpr (debug_move) {
                                std::cout << "endmv = " << endmv << " DOES" << (isect2 ? "" : " NOT") << " land in next tri\n";
                            }
                            if (isect2) {
                                // We DID land in the neighbouring triangle. We are done.
                                cam_to_surface = reorient_model * cam_to_surface;
                                flags.set (cmm_fl::done, true);
                            } else {
                                // Incomplete; We've sailed past newtv_sf.  We need to
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

                        ti0 = _ti;
                        ne.tris.push_back (ti0);
                        tn0 = _tn;

                    } else {
                        // other triangle not found?! We probably went off the edge of our navigation model mesh
                        flags.set (cmm_fl::done, true);
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
                            std::cout << "\nCompare current location " << cam_to_scene.translation()
                                      << " with ti0: " << this->triangle_vertices (ti0, model_to_scene) << "\n\n";
                            ne.m_type = NavException::type::mv_to_vertex;
                            throw ne;
                        }
                    } else {
                        // Test if it was movement-within; the simplest case
                        if constexpr (debug_move) {
                            std::cout << "No cross point and not colinear.\n  Testing if "
                                      << (hov_sf + mv_inplane + (tn0 / 2.0f)) << " intersects tv_sf (" << tv_sf << ") dirn "
                                      << -tn0 << "...\n";
                        }
                        auto [single_mv, he] = sm::algo::ray_tri_intersection<float, true, true, true> (tv_sf[0], tv_sf[1], tv_sf[2], hov_sf + mv_inplane + (tn0 / 2.0f), -tn0);
                        flags.set (cmm_fl::single_movement, single_mv);
                    }

                    if (flags.test (cmm_fl::single_movement)) {
                        if constexpr (debug_move) { std::cout << "End of movement is *still* in ti0, so move mv_inplane/mv_camframe\n"; }
                        // Perform simplest movement, which is just to translate by mv_inplane
                        cam_to_surface.pretranslate (mv_inplane);
                        flags.set (cmm_fl::done, true);

                    } else {
                        if constexpr (debug_move) {
                            std::cout << "End of movement is NOT in " << ti0[0] << "," << ti0[1] << "," << ti0[2] << ". Look for start neighbours\n";
                        }
                        // Test 3 neighbours across the edges to find any for which the start location is also within-boundary
                        std::array<uint32_t, 4> _ti_2n = { std::numeric_limits<uint32_t>::max() };
                        sm::vec<float>_tn_2n = {};
                        for (uint32_t i = 0u; i < 3u; i++) {
                            uint32_t i1 = i;
                            uint32_t i2 = (i + 1) % 3u;
                            auto [_ti, _tn] = this->find_other_triangle_containing (ti0[i1], ti0[i2], ti0);
                            if (_ti[0] != std::numeric_limits<uint32_t>::max()) {
                                // Test to see if start location was inside a neighbour
                                sm::vec<sm::vec<float>, 3> tv_nb = this->triangle_vertices (_ti, model_to_scene);
                                _tn = this->triangle_normal (tv_nb);

                                auto [is, h] = sm::algo::ray_tri_intersection<float, true, true, true> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf, -_tn);
                                sm::vec<float> mv_orthog_nb = _tn * (mv_sf.dot (_tn) / (_tn.dot(_tn)));
                                sm::vec<float> mv_inplane_nb = mv_sf - mv_orthog_nb;
                                auto [endis, endh] = sm::algo::ray_tri_intersection<float, true, true, true> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf + mv_inplane_nb, -_tn);
                                if constexpr (debug_move) {

                                    std::cout << "Start of move " << (is ? "IS" : "is NOT")
                                              << " in " << _ti[0] << "," << _ti[1] << "," << _ti[2] << " / " <<  tv_nb << std::endl;
                                    std::cout << "End of move " << (endis ? "IS" : "is NOT")
                                              << " in " << _ti[0] << "," << _ti[1] << "," << _ti[2] << " / " <<  tv_nb << std::endl;
                                }

                                // Here, start is in original, end may not be in original. This
                                // is an 'intersection detected crossing' of a triangle edge
                                // which wasn't picked up with compute_crossing_location
                                if (endis) {
                                    // End is in neighbour so this is a detected crossing
                                    if constexpr (debug_move) { std::cout << "DETECTED crossing! Pass on to next loop!\n"; }
                                    flags.set (cmm_fl::detected_crossing, true);
                                    detected_edge = { ti0[i1], ti0[i2] };
                                    detected_edgevec = tv_nb[i2] - tv_nb[i1];
                                    break; // out of for
                                } else { // end not in neighbour
                                    if (is) { // start is in neighbour tri (will re-orient to this and re-loop)
                                        _ti_2n = _ti;
                                        _tn_2n = _tn;
                                        break; // out of for
                                    } else {
                                        // end is not in neighbour, and neither is start. This occurs if the end
                                        // is ON the boundary, but precision errors mean it isn't 'in' either
                                        // start or neighbour. Assume on edge? Push by epsilon?
                                        std::cout << "Maybe end is right on the boundary and precision errors mean it isn't 'in' either start of neighbour?\n";
                                        auto [endisplus, endh] = sm::algo::ray_tri_intersection<float> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf + (mv_inplane_nb * 1.0001f), -_tn);
                                        if (endisplus) {
                                            std::cout << "Just a little further IS in neighbour\n";
                                        } else {
                                            std::cout << "A little further ISN'T in neighbour\n";
                                        }
                                        ne.m_type = NavException::type::undetected_crossing;
                                        throw ne;
                                    }
                                }
                            }
                        }

                        if (_ti_2n[0] != std::numeric_limits<uint32_t>::max()) {
                            // Now we know an alternative start triangle for the movement. Re-orient to this and re-loop
                            ti0 = _ti_2n;
                            ne.tris.push_back (ti0);
                            tn0 = _tn_2n;
                            // recompute mv_inplane for this neighbour triangle
                            mv_orthog = tn0 * (mv_sf.dot (tn0) / (tn0.dot (tn0)));
                            mv_inplane = mv_sf - mv_orthog; // sf
                        } else if (flags.test (cmm_fl::detected_crossing)) {
                            // We didn't find an alternative start triangle, but we did detect an edge crossing by intersection, so continue.
                        } else {
                            // Just had this, agent appears close to vertex of ti0
                            std::cout << "\nCompare current location " << cam_to_scene.translation()
                                      << " with ti0: " << this->triangle_vertices (ti0, model_to_scene) << "\n\n";
                            ne.m_type = NavException::type::undetected_crossing;
                            throw ne;
                        }

                    } // single movement if/else

                } // compute_crossing_location if/else

            } // triangle traversing while loop

            // Raise cam_to_surface up by hoverheight and then return
            cam_to_surface.pretranslate (hoverheight * tn0);
            if constexpr (debug_move) {
                std::cout << "looping mv_inplanes completed. Final camloc_sf: "
                          << cam_to_surface.translation()
                          << std::endl;
            }
            return cam_to_surface;

        } // compute_mesh_movement

    }; // struct NavMesh

} // namespace
