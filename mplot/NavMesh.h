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
#include <sm/mat>
#include <sm/geometry>
#include <sm/util>

namespace mplot
{
    namespace mesh
    {
        /*
         * Half-edge data structures for ordered meshes
         */
        template<typename I = uint32_t> requires std::is_integral_v<I>
        struct halfedge
        {
            // two vertex indices for start and end of this halfedge
            sm::vec<I, 2> vi = { std::numeric_limits<I>::max(), std::numeric_limits<I>::max() };
            I twin = std::numeric_limits<I>::max(); // twin half edge
            I next = std::numeric_limits<I>::max(); // next half edge in face (or hole)
            I prev = std::numeric_limits<I>::max(); // prev half edge in face (or hole)
            I flags = 0; // 0x1: boundary halfedge
        };

        template<typename I = uint32_t, typename F=float, I N = 3> requires std::is_integral_v<I>
        struct vertex
        {
            // Coordinate position of vertex
            sm::vec<F, N> p = {};
            // A halfedge (hi: halfedge index) emanating from this vertex
            I hi = std::numeric_limits<I>::max();
        };

        template<typename I = uint32_t> requires std::is_integral_v<I>
        struct face
        {
            // The index of the starting halfedge that records the existence of this face
            I hi = std::numeric_limits<I>::max();
        };
    }

    // Exception that (used to) return triangles that were near the location of the error
    struct NavException : public std::exception
    {
        enum class type : uint32_t { generic, no_intersection, zero_mv, mv_to_vertex, undetected_crossing, nan_mv, off_edge };

        NavException (const type _type) : m_type(_type) {}

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
        std::vector<mesh::vertex<>> vertex = {};

        /*!
         * The vector of half edges in the mesh
         */
        std::vector<mesh::halfedge<>> halfedge = {};

        /*!
         * Triangle mesh faces. populated by VisualModel::make_navmesh()
         */
        std::vector<mesh::face<>> triangles = {};

        //! Holds a copy of the bb of the parent model
        sm::range<sm::vec<float>> bb;

        //! When navigating, this is the 'current triangle' that you're located over/near
        uint32_t ti0 = std::numeric_limits<uint32_t>::max();

        void save (const std::string& filename) const
        {
            std::cout << "Save NavMesh to " << filename << std::endl;

            std::ofstream fout (filename, std::ios::binary | std::ios::out | std::ios::trunc);
            if (!fout.is_open()) {
                std::cerr << "NavMesh::save: Failed to open " << filename << " for writing, continue\n";
                return;
            }
            // fout is open
            uint64_t vertex_sz = this->vertex.size();
            uint64_t halfedge_sz = this->halfedge.size();
            uint64_t triangles_sz = this->triangles.size();

            // Write sizes at head of file, as the first thing
            sm::util::binary_write (fout, vertex_sz);
            sm::util::binary_write (fout, halfedge_sz);
            sm::util::binary_write (fout, triangles_sz); // 3 * 8 = 24 bytes

            // Write the bb range next.
            sm::util::binary_write (fout, this->bb.min);
            sm::util::binary_write (fout, this->bb.max); // 2 * 3 * 4 = 24 bytes

            // Now loop
            for (auto v : this->vertex) {
                sm::util::binary_write (fout, v.p);
                sm::util::binary_write (fout, v.hi);     // 3 * 4 + 4 = 16 bytes per line
            }

            for (auto he : this->halfedge) {
                sm::util::binary_write (fout, he.vi);
                sm::util::binary_write (fout, he.twin);
                sm::util::binary_write (fout, he.next);
                sm::util::binary_write (fout, he.prev);  // 5 * 4 = 20 bytes per line
                sm::util::binary_write (fout, he.flags);  // plus 4
            }

            for (auto t : this->triangles) { sm::util::binary_write (fout, t.hi); }
        }

        void load (const std::string& filename)
        {
            std::cout << "Load NavMesh from " << filename << std::endl;

            std::ifstream fin (filename, std::ios::binary | std::ios::in);
            if (!fin.is_open()) { throw std::runtime_error ("NavMesh::load: Failed to open file"); }

            uint64_t vertex_sz = 0;
            uint64_t halfedge_sz = 0;
            uint64_t triangles_sz = 0;

            sm::util::binary_read (fin, vertex_sz);
            sm::util::binary_read (fin, halfedge_sz);
            sm::util::binary_read (fin, triangles_sz); // 3 * 8 = 24 bytes

            sm::util::binary_read (fin, this->bb.min);
            sm::util::binary_read (fin, this->bb.max);

            this->vertex.resize (vertex_sz);
            this->halfedge.resize (halfedge_sz);
            this->triangles.resize (triangles_sz);

            for (auto& v : this->vertex) {
                sm::util::binary_read (fin, v.p);
                sm::util::binary_read (fin, v.hi);     // 3 * 4 + 4 = 16 bytes per line
            }

            for (auto& he : this->halfedge) {
                sm::util::binary_read (fin, he.vi);
                sm::util::binary_read (fin, he.twin);
                sm::util::binary_read (fin, he.next);
                sm::util::binary_read (fin, he.prev);  // 5 * 4 = 20 bytes per line
                sm::util::binary_read (fin, he.flags);
            }

            for (auto& t : this->triangles) { sm::util::binary_read (fin, t.hi); }
        }

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
                sm::vec<float> vcoord = (viewmatrix * this->vertex[j].p).less_one_dim();
                float d = (scene_coord - vcoord).length();
                if (d < min_d) {
                    min_d = d;
                    i = j;
                }
            }
            return i;
        }

        // Return the three vertices for the triangle specified as three indices into NavMesh::vertex
        sm::vec<sm::vec<float>, 3> triangle_vertices (uint32_t tri_hi) const
        {
            sm::vec<sm::vec<float>, 3> trivert = {};
            if (tri_hi == std::numeric_limits<uint32_t>::max()) {
                std::cout << "tri_hi is null?\n";
                return trivert;
            }

            uint32_t i = 0;
            uint32_t hi = tri_hi;
            do {
                //std::cout << "vertex.size(): " << this->vertex.size() << std::endl;
                if (this->halfedge[hi].vi[0] < this->vertex.size()) {
                    trivert[i] = this->vertex[this->halfedge[hi].vi[0]].p;
                }
                ++i;
                hi = this->halfedge[hi].next;
            } while (hi != tri_hi);
            return trivert;
        }

        // Return the three vertices for the triangle specified as three indices into NavMesh::vertex transformed by transform
        sm::vec<sm::vec<float>, 3> triangle_vertices (uint32_t tri_hi, const sm::mat<float, 4>& transform) const
        {
            sm::vec<sm::vec<float>, 3> trivert = {};
            if (tri_hi == std::numeric_limits<uint32_t>::max()) {
                std::cout << "tri_hi is null?\n";
                return trivert;
            }

            uint32_t i = 0;
            uint32_t hi = tri_hi;
            do {
                //std::cout << "vertex.size(): " << this->vertex.size() << std::endl;
                if (this->halfedge[hi].vi[0] < this->vertex.size()) {
                    trivert[i] = (transform * this->vertex[this->halfedge[hi].vi[0]].p).less_one_dim();
                }
                ++i;
                hi = this->halfedge[hi].next;
            } while (hi != tri_hi);
            return trivert;
        }

        // Compute the triangle normal for the ordered triplet of triangle vertices, tverts
        sm::vec<float, 3> triangle_normal (const sm::vec<sm::vec<float>, 3>& tverts) const
        {
            sm::vec<float> n = (tverts[1] - tverts[0]).cross (tverts[2] - tverts[0]);
            n.renormalize();
            return n;
        }

        // Retrieve the halfedge as a vector, transformed by the given transform
        sm::vec<float> edge_vector (uint32_t hi, const sm::mat<float, 4>& transform) const
        {
            const sm::vec<float> v0 = (transform * this->vertex[this->halfedge[hi].vi[0]].p).less_one_dim();
            const sm::vec<float> v1 = (transform * this->vertex[this->halfedge[hi].vi[1]].p).less_one_dim();
            return v1 - v0;
        }

        // Find all the neighbours of triangle *vertex* index a.
        // \return vector of halfedge indices
        std::vector<uint32_t>
        find_neighbours (uint32_t a) const
        {
            uint32_t hi = a;
            std::vector<uint32_t> rtn = {};
            do {
                // hi emanates from the vertex, so return it.
                rtn.push_back (hi);
                hi = this->halfedge[this->halfedge[hi].prev].twin;
                // or hi = this->halfedge[this->halfedge[hi].twin].next; // Clockwise
            } while (hi != a);
            return rtn;
        }

        /*
         * After making the neighbour relations from the OpenGL mesh, the last step is to fill in
         * the boundary halfedges. Find all halfedges with an unset twin and then start creating the
         * new half edges to fill in.
         */
        void add_boundary_halfedges()
        {
            constexpr uint32_t max = std::numeric_limits<uint32_t>::max();
            constexpr bool debug_bnd = true;

            const uint32_t sz = this->halfedge.size();
            uint32_t j = 0;
            for (uint32_t i = 0; i < sz; ++i) {
                if (this->halfedge[i].twin == max) {
                    // This halfedge does not have a twin, walk the boundary from here
                    const uint32_t j0 = j; // j index at boundary start
                    uint32_t bprev = max;
                    uint32_t cur = i;
                    uint32_t done = 0u;
                    while (!done) {
                        if constexpr (debug_bnd) {
                            std::cout << "                     Search for boundary from cur = " << cur << std::endl;
                        }
                        uint32_t bcand = cur; // bcand starts as an internal halfedge
                        uint32_t bcandi = max;
                        uint32_t counter = 0u;
                        if constexpr (debug_bnd) { std::cout << "halfedge[i].twin = " << this->halfedge[i].twin << std::endl; }
                        //uint32_t bcand0 = this->halfedge[this->halfedge[bcand].prev].twin;
                        //uint32_t bcand0t = max;
                        do {
                            bcandi = this->halfedge[bcand].prev;
                            if constexpr (debug_bnd) { std::cout << "bcandi: " << bcandi << std::endl; }
                            bcand = this->halfedge[bcandi].twin; // if max, it's a boundary, else it's internal
                            if constexpr (debug_bnd) { std::cout << "bcand: " << bcand << std::endl; }
                            if (counter++ > 6) {
                                std::cout << "Something is wrong; returning\n";
                                return;
                            }
                            //if (counter == 1) { bcand0t = bcand0; }

                        } while (bcand != max // halfedge[i].twin is usually max, so second condition may be sufficient
                                 && bcand != this->halfedge[i].twin
                                 //&& bcand != bcand0t
                                 && bcandi != cur
                            );

                        this->halfedge.push_back ({{this->halfedge[cur].vi[1], this->halfedge[cur].vi[0]}, cur, bprev, sz + j + 1, 1});
                        this->halfedge[cur].twin = sz + j;

                        if (bcandi == i) {
                            this->halfedge[sz + j0].prev = sz + j - 1;
                            ++done;
                        } else {
                            bprev = sz + j;
                            cur = bcandi;
                            ++j;
                        }
                    }
                    if constexpr (debug_bnd) {
                        std::cout << "Added " << (j - j0) << " halfedges to that boundary\n";
                    }
                }
            }
            if constexpr (debug_bnd) { std::cout << __func__ << " returning\n"; }
        }

        /*
         * Determine neighbour relations. That means populating a halfedge data structure. Don't
         * think there's any way around the at-worst O(N^2) computation, so save results into an h5
         * file that can be loaded at startup.
         *
         * The key is the half-edge data structure.
         * See: https://jerryyin.info/geometry-processing-algorithms/half-edge/
         */
        void compute_neighbour_relations()
        {
            constexpr bool debug_nr = false;
            uint32_t sz = this->halfedge.size();
            if constexpr (debug_nr) { std::cout << "Finding twins for " << sz << " halfedge\n"; }

            // Search a 'band' either side of i first, assuming that neighbour faces are likely
            // to have been nearby in the indices array
            const uint32_t band = 3 * 1000;

            uint32_t wider_searches = 0; // Count how many times we make a wider search

            uint64_t twin_meandist = 0; // See how far a search has to search for a twin
            uint32_t twins = 0;

            for (uint32_t i = 0; i < sz; ++i) {

                const sm::vec<uint32_t, 2>& vi = this->halfedge[i].vi;

                // halfedge[i].twin may already have been set (as we set two twins at a time)
                if (this->halfedge[i].twin != std::numeric_limits<uint32_t>::max()) { continue; }

                // It's useful to know how long you will have to wait...
                if (i % 20000u == 0u) { std::cout << ((100.0f * i)/sz) << " percent...\n" << std::endl; }

                uint32_t sb = i >= band ? i - band : 0;
                uint32_t eb = i + band < sz ? i + band : sz;

                // First sb to eb, which we hope is most likely to find a twin
                for (uint32_t j = sb; j < eb; ++j) {
                    if (j == i) { continue; }
                    const sm::vec<uint32_t, 2>& vij = this->halfedge[j].vi;
                    if (vi[0] == vij[1] && vi[1] == vij[0]) { // It's a match
                        this->halfedge[i].twin = j;
                        this->halfedge[j].twin = i;
                        break;
                    }
                }

                uint32_t wider = 0;
                if (this->halfedge[i].twin == std::numeric_limits<uint32_t>::max()) {
                    // Then, if no match, search from 0 to sb
                    if (sb != 0 && !wider) { wider = 1; }
                    for (uint32_t j = 0; j < sb; ++j) {
                        if (j == i) { continue; }
                        const sm::vec<uint32_t, 2>& vij = this->halfedge[j].vi;
                        if (vi[0] == vij[1] && vi[1] == vij[0]) { // It's a match
                            this->halfedge[i].twin = j;
                            this->halfedge[j].twin = i;
                            break;
                        }
                    }
                }

                if (this->halfedge[i].twin == std::numeric_limits<uint32_t>::max()) {
                    // If still no match search from eb to sz
                    if (eb != sz && !wider) { wider = 1; }
                    for (uint32_t j = eb; j < sz; ++j) {
                        if (j == i) { continue; }
                        const sm::vec<uint32_t, 2>& vij = this->halfedge[j].vi;
                        if (vi[0] == vij[1] && vi[1] == vij[0]) { // It's a match
                            this->halfedge[i].twin = j;
                            this->halfedge[j].twin = i;
                            break;
                        }
                    }
                }

                wider_searches += wider;

                if (this->halfedge[i].twin != std::numeric_limits<uint32_t>::max()) {
                    if (wider) {
                        twin_meandist += i > this->halfedge[i].twin ? i - this->halfedge[i].twin : this->halfedge[i].twin - i;
                        ++twins;
                    }
                } // else halfedge[i] is an edge of the mesh
            }
            if constexpr (debug_nr) {
                std::cout << "In " << sz << " halfedge searches, had to widen the search in "
                          << (100.0 * wider_searches) / sz << " percent\n";
                std::cout << "Mean wider twin search distance (in array elements) was "
                          << static_cast<double>(twin_meandist) / twins << "\n";
            }
        }

        /*
         * Find the location, and the triangle indices at which a ray starting from coord with
         * direction vdir - the 'penetration point' intersects with this NavMesh model. The length
         * of vdir is used to avoid finding the intersection at the 'back' of the model.
         *
         * \param model_to_scene Transform that is only passed to find_vertex_normal. May in future be unnecessary.
         *
         * \param ti_ml The most likely triangle, if you know what it probably is, to reduce the
         * search time.
         *
         * \return a tuple containing crossing location, halfedge index (which specifies a triangle)
         */
        std::tuple<sm::vec<float>, uint32_t>
        find_triangle_crossing (const sm::vec<float>& coord_mf, const sm::vec<float>& vdir,
                                const sm::mat<float, 4>& model_to_scene,
                                const uint32_t ti_ml = std::numeric_limits<uint32_t>::max() ) const
        {
            constexpr bool debug_ftc = false;
            constexpr float fmax = std::numeric_limits<float>::max();
            sm::vec<float> vstart = coord_mf - (vdir / 2.0f);

            // Return objects
            sm::vec<float> isect_p = { fmax, fmax, fmax };
            uint32_t isect_ti = std::numeric_limits<uint32_t>::max();

            float isect_d = std::numeric_limits<float>::max(); // distance to intersect

            const float vdsos = vdir.sos();

            // Have we been passed a 'most likely triangle' to test first? If so, test it.
            if (ti_ml != std::numeric_limits<uint32_t>::max()) {
                sm::vec<sm::vec<float>, 3> v = this->triangle_vertices (ti_ml);
                auto [isect, p] = sm::geometry::ray_tri_intersection<float, float, true, false> (v[0], v[1], v[2], vstart, vdir);
                if (isect) {
                    float d = (p - vstart).sos();
                    if (d < vdsos) {
                        isect_p = p;
                        isect_ti = ti_ml;
                        isect_d = d;
                    }
                }
                if (isect_d != std::numeric_limits<float>::max()) {
                    // we found it in the first triangle!
                    return { isect_p, isect_ti };
                }

                // Next, test the neighbours of ti_ml
                std::vector<uint32_t> nbs = this->find_neighbours (ti_ml);
                for (uint32_t nb : nbs) {
                    v = this->triangle_vertices (nb);
                    auto [isect, p] = sm::geometry::ray_tri_intersection<float, float, true, false> (v[0], v[1], v[2], vstart, vdir);
                    if (isect) {
                        float d = (p - vstart).sos();
                        if (d < vdsos) {
                            isect_p = p;
                            isect_ti = ti_ml;
                            isect_d = d;
                        }
                    }

                    if (isect_d != std::numeric_limits<float>::max()) {
                        return { isect_p, isect_ti };
                    }
                }
            }

            // Fall back to testing ALL the triangles...
            for (auto tri : this->triangles) {

                if constexpr (debug_ftc) {
                    std::cout << "this->halfedge["<< 0 << "] " << (&this->halfedge[0]) << " contains: vi:"
                              <<  this->halfedge[0].vi
                              << ", twin:" << this->halfedge[0].twin
                              << ", next:" << this->halfedge[0].next
                              << ", prev:" << this->halfedge[0].prev << std::endl;
                    std::cout << "CF. Passing tri.he " << tri.hi << " to triangle_vertices(): with he->vi =  " << this->halfedge[tri.hi].vi << std::endl;
                }

                sm::vec<sm::vec<float>, 3> v = this->triangle_vertices (tri.hi);
                auto [isect, p] = sm::geometry::ray_tri_intersection<float, float, true, false> (v[0], v[1], v[2], vstart, vdir);
                // What if the triangle is one on the *other side of the model*?? Have to use
                // vdir.sos() to exclude those that are too far and the distance^2 to find the
                // closest one that isn't.
                if (isect) {
                    float d = (p - vstart).sos();
                    if (d < isect_d && d < vdsos) {
                        isect_p = p;
                        isect_ti = tri.hi;
                        isect_d = d;
                    }
                }
            }

            if (isect_p[0] == fmax) {
                // Found no triangle intersection; check vertices, in case vdir points perfectly at a vertex.
                for (uint32_t vi = 0; vi < this->vertex.size(); ++vi) {
                    sm::vec<float> vertex_n = this->find_vertex_normal (this->vertex[vi].hi, model_to_scene);
                    vertex_n.renormalize();
                    vstart = coord_mf + (vertex_n / 2.0f);
                    if (sm::geometry::ray_point_intersection (this->vertex[vi].p, vstart, -vertex_n)) {
                        float d = (this->vertex[vi].p - vstart).sos();
                        if (d < isect_d && d < vdir.sos()) {
                            if constexpr (debug_ftc) { std::cout << "Register vertex triangle_crossing\n"; }
                            isect_p = this->vertex[vi].p;
                            isect_ti = this->vertex[vi].hi;
                            isect_d = d;
                        }
                    }
                }
            }

            return { isect_p, isect_ti };
        }

        // Find the location, and the triangle indices (by means of a halfedge index) at which a ray
        // between coord (in model frame) and the model centroid cross - the 'penetration point'.
        std::tuple<sm::vec<float>, uint32_t>
        find_triangle_crossing (const sm::vec<float>& coord_mf, const sm::mat<float, 4>& model_to_scene) const
        {
            sm::vec<float> vdir = this->bb.mid() - coord_mf;
            vdir.renormalize();
            return this->find_triangle_crossing (coord_mf, vdir, model_to_scene);
        }

        // Find the normal of the vertex specified by halfedge vhe
        sm::vec<float> find_vertex_normal (const uint32_t ti, const sm::mat<float, 4>& transform) const
        {
            auto neighbs = this->find_neighbours (ti);
            sm::vec<float> vn = {};
            if (neighbs.size() == 0) { return vn; }
            for (auto nb : neighbs) {
                // Turn nb, a half edge index, into a triangle?
                vn += this->triangle_normal (this->triangle_vertices (nb, transform));
            }
            return (vn / neighbs.size());
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
            // The crossed halfedge
            uint32_t halfedge = std::numeric_limits<uint32_t>::max();
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
         * \param t_verts *Ordered* vertices of the triangle. Vertices should be in anti-clockwise
         * order when viewed with the triangle normal vector coming 'out of the page'. These should
         * be the vertices that were generated with the param tri (using function
         * triangle_vertices()).  Could be obtained within this function, but have already been
         * computed, and they may be in a different frame of ref that they have in this->vertex
         *
         * \param tri The halfedge that gives the triangle
         *
         * \param mv_s The start of the planned movement
         *
         * \param mv_inplane The planned movement
         */
        crossing_data compute_crossing_location (const sm::vec<sm::vec<float>, 3>& t_verts,
                                                 const uint32_t tri,
                                                 const sm::vec<float>& mv_s,
                                                 const sm::vec<float>& mv_inplane)
        {
            constexpr bool debug = false;
            crossing_data cd;
            cd.pm.flags.set (pm_fl::no_cross_point, true);

            sm::vec<float> p = mv_s + mv_inplane;
            sm::vec<float> tn = this->triangle_normal (t_verts);

            // do-while with tri
            uint32_t hi = tri;
            uint32_t a = 0;
            sm::vec<bool, 3> inside = { false, false, false };
            do {
                uint32_t b = (a + 1u) % 3u;

                sm::vec<float> edge = t_verts[b] - t_verts[a];
                sm::vec<float> ptoe = p - t_verts[a];

                inside[a] = (tn.dot (edge.cross (ptoe)) >= 0);
                if (!inside[a]) {
                    partial_movement pm = find_edge_crossing (t_verts[a], t_verts[b], tn, mv_s, mv_inplane);
                    if constexpr (debug) {
                        if (pm.flags.test (pm_fl::colinear)) {
                            std::cout << "ccl: fec returned pm.colinear true for t" << a << "t" << b << "\n";
                        }
                    }
                    if (pm.flags.test (pm_fl::no_cross_point)
                        && pm.flags.test (pm_fl::colinear) == false) {
                        inside[a] = true;
                        if constexpr (debug) {
                            std::cout << "ccl: No intersection for edge t" << a << "t" << b << " " << t_verts[a] << " -- " << t_verts[b]
                                      << " and move " << mv_s << " -- " << (mv_s + mv_inplane) << std::endl;
                        }
                    } else {
                        if constexpr (debug) {
                            if (pm.flags.test (pm_fl::colinear)) { std::cout << "ccl: colinear t0t1\n"; }
                            std::cout << "ccl: Intersection for edge t" << a << "t" << b << " " << t_verts[a] << " -- " << t_verts[b]
                                      << " and move " << mv_s << " -- " << (mv_s + mv_inplane) << std::endl;
                        }
                        cd.pm = pm;
                        cd.tri_edge = edge;
                        cd.halfedge = hi;
                    }
                }

                ++a;
                hi = this->halfedge[hi].next;

            } while (hi != tri && a < 3);


            // We've now tested edge crossing for three edges in the triangle.
            if constexpr (debug) {
                if (cd.pm.flags.test (pm_fl::no_cross_point) == false) {
                    std::cout << "ccl: Crossed over" << (inside[0] ? " " : " 0-1")
                              << (inside[1] ? " " : " 2-1") <<  (inside[2] ? " " : " 0-2") << std::endl;
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
                    std::cout << "ccl: No crossings " << (inside[0] ? " " : "!!0-1")
                              << (inside[1] ? " " : "!!2-1") <<  (inside[2] ? " " : "!!0-2") << std::endl;
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
         * \return tuple containing: the hit point in scene coordinates; --the triangle normal of the
         * triangle we hit;-- and the indices of the triangle we hit.
         */
        std::tuple<sm::vec<float>, uint32_t>
        find_triangle_hit (const sm::mat<float, 4>& model_to_scene,
                           const sm::vec<float>& camloc_mf, const sm::vec<float>& vdir,
                           uint32_t ti_ml = std::numeric_limits<uint32_t>::max())
        {
            this->ti0 = std::numeric_limits<uint32_t>::max();
            sm::vec<float> hit = {};
            // Want to pass 'best tri' to this
            std::tie (hit, this->ti0) = this->find_triangle_crossing (camloc_mf, vdir, model_to_scene, ti_ml);

            if (this->ti0 == std::numeric_limits<uint32_t>::max()) { std::cout << __func__ << ": No hit\n"; }

            sm::vec<float> hp_scene = (model_to_scene * hit).less_one_dim();

            constexpr bool debug = false;
            if constexpr (debug) {
                std::cout << "found hit at " << hit << " (model); " << hp_scene << " (scene) in direction " << vdir << "\n";
                // Check we'll get a hit when we compute_mesh_movement:
                sm::vec<sm::vec<float>, 3> tv_mf = this->triangle_vertices (this->ti0);
                auto tn = this->triangle_normal (tv_mf);
                std::cout << "tn: " << tn << ", length " << tn.length() << std::endl;
                std::cout << "TEST ray_tri_intersection (hit,-tn): " << (hit + (tn / 2.0f)) << "," << -tn << std::endl;
                auto [isect, hov_mf] = sm::geometry::ray_tri_intersection<float> (tv_mf[0], tv_mf[1], tv_mf[2], hit + (tn / 2.0f), -tn);
                if (isect) {
                    std::cout << "ray_tri_intersection confirms we would hit at " << hov_mf << "\n";
                } else {
                    std::cout << "ray_tri_intersection DOES NOT get a hit\n";
                }
            }

            return { hp_scene, this->ti0 };
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
         * \return tuple containing: the hit point in scene coordinates and the mesh::face we hit.
         */
        std::tuple<sm::vec<float>, uint32_t>
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
                                           const sm::vec<float>& _x, const sm::vec<float>& _y, const sm::vec<float>& _z,
                                           const float hoverheight)
        {
            // I think this positions correctly now (which is all it has to do). It ignores scaling
            // in model_to_scene. Can be reduced to use fewer mat<>s.
            sm::mat<float, 4> cam_mv_y;
            cam_mv_y.translate (sm::vec<float>{0, hoverheight, 0});

            // The basis _x, _y, _z, where these are vectors in the model frame that define a camera frame
            sm::mat<float, 4> cam_to_model_rotn = sm::mat<float, 4>::frombasis (_x, _y, _z);
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
            if (this->ti0 == std::numeric_limits<uint32_t>::max()) {
                std::cout << __func__ << ": No hit/triangle normal\n";
                return sm::mat<float, 4>{};
            }

            // Place the camera on the model, and orient it randomly in the 'model plane'
            // The camera frame always has y up. Choose a random vector in the plane for 'x'
            // and then set z from this random x and the triangle norm (y).
            sm::vec<float> rand_vec;
            rand_vec.randomize();
            sm::vec<sm::vec<float>, 3> tv_sf = this->triangle_vertices (this->ti0, model_to_scene);
            sm::vec<float> tn = this->triangle_normal (tv_sf);
            sm::vec<float> _x = rand_vec.cross (tn);
            _x.renormalize();
            sm::vec<float> _z = _x.cross (tn);

            return this->position_camera (hp_scene, model_to_scene, _x, tn, _z, hoverheight);
        }

        /*!
         * A version of position camera that aligns the camera direction (i.e. where it is looking - its 'forwards')
         * as closely as possible with the passed-in vector
         */
        sm::mat<float, 4> position_camera (const sm::vec<float>& hp_scene, const sm::mat<float, 4>& model_to_scene,
                                           const float hoverheight, const sm::vec<float>& fwds)
        {
            // Let's 'draw' the camera towards the model and then arrange its normal upwards wrt to the normal of the model.
            if (this->ti0 == std::numeric_limits<uint32_t>::max()) {
                std::cout << __func__ << ": No hit/triangle normal\n";
                return sm::mat<float, 4>{};
            }

            // Project fwds onto the plane tn
            sm::vec<sm::vec<float>, 3> tv_sf = this->triangle_vertices (this->ti0, model_to_scene);
            sm::vec<float> tn = this->triangle_normal (tv_sf);
            sm::vec<float> _z = sm::geometry::vector_plane_projection (tn, fwds);
            _z.renormalize();
            sm::vec<float> _x = -_z.cross (tn);
            _x.renormalize();

            return this->position_camera (hp_scene, model_to_scene, _x, tn, _z, hoverheight);
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

            // Boolean state flags used in this function
            enum class cmm_fl : uint32_t { done, detected_crossing, single_movement, vertex_crossing };
            sm::flags<cmm_fl> flags;

            // Camera location, scene frame
            sm::vec<float> camloc_sf = cam_to_scene.translation();
            // Convert indices to vertices for triangle ti0, converting to the scene frame
            sm::vec<sm::vec<float>, 3> tv_sf = this->triangle_vertices (this->ti0, model_to_scene);
            // Compute the triangle normal in the scene frame
            sm::vec<float> tn0 = this->triangle_normal (tv_sf);

            if constexpr (debug_move) {
                std::cout << "\n# compute_mesh_movement:\n"
                          << "\nti0: " << this->ti0
                          << "\nti0 (sf): " << tv_sf << "\nnormal " << tn0
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
                std::cout << "First ray_tri_intersection (raystart,-tn0): "
                          << (camloc_sf + (tn0 / 2.0f)) << "," << -tn0 << std::endl;
            }
            bool isect = false;
            sm::vec<float, 3> hov_sf = {};
            std::tie (isect, hov_sf) = sm::geometry::ray_tri_intersection<float> (tv_sf[0], tv_sf[1], tv_sf[2], camloc_sf + (tn0 / 2.0f), -tn0);

            // Use the detected location, hov_sf to compute the surface location of the camera - its 'hover location'
            sm::mat<float, 4> cam_to_surface = cam_to_scene;
            cam_to_surface.pretranslate (hov_sf - camloc_sf); // This is now our init pose; the camera is now at the surface

            // Try double precision
            if (isect == false) {
                std::tie (isect, hov_sf) = sm::geometry::ray_tri_intersection<float, double> (tv_sf[0], tv_sf[1], tv_sf[2], camloc_sf + (tn0 / 2.0f), -tn0);
                if constexpr (debug_move) {
                    if (isect == false) {
                        std::cout << "No isect at start with ti0 using float OR double internally" << std::endl;
                    } else {
                        std::cout << "Intersection at start with ti0 using *double* internally" << std::endl;
                    }
                }
            }

            // If that didn't work, try the triangle *vertices*
            uint32_t int_vertex_hi = std::numeric_limits<uint32_t>::max(); // intersection vertex
            if (isect == false) {
                if constexpr (debug_move) { std::cout << "Try the triangle vertices...\n"; }
                uint32_t hi = this->ti0;
                uint32_t i = 0;
                do {
                    // We need to use the *vertex* normal for this test - the average of all the adjacent triangle normals!
                    sm::vec<float> vertex_n = this->find_vertex_normal (hi, model_to_scene);
                    vertex_n.renormalize();
                    // How to figure out tv_sf[i]?
                    // sm::vec<float> v_sf = (model_to_scane * this->vertex[halfedge[hi].vi[0]]).less_one_dim();
                    // or, if tv_sf[0] is the start of ti0, then we can do this:
                    if (sm::geometry::ray_point_intersection (tv_sf[i], camloc_sf + (vertex_n / 2.0f), -vertex_n)) {
                        if constexpr (debug_move) {
                            std::cout << "A VERTEX intersection is the start at " << tv_sf[i] << ", compare this with hov_sf = " << hov_sf << "\n";
                            // if start is vertex, need to check movement across all the triangle-neighbours of this vertex (see later use of int_vertex_hi)
                        }
                        hov_sf = tv_sf[i];
                        int_vertex_hi = hi;
                        isect = true;
                    }
                    ++i;
                    hi = this->halfedge[hi].next;

                } while (hi != this->ti0);
            }

            if (isect == true) {
                if constexpr (debug_move) { std::cout << "First ray_tri_intersected. Start of move is IN triangle ti0\n"; }
            } else {
                if constexpr (debug_move2) {
                    std::cout << "No intersection (at start) with triangle ti0, check neighbours (and maybe update ti0)" << std::endl;
                }

                // When very close to the boundary, ray_tri_intersection may fail. This triggers a
                // search for a neighbouring triangle which the camera may instead be hovering over
                // (this can occur when moving along an edge)
                uint32_t hi = this->ti0;
                do {
                    uint32_t twin = this->halfedge[hi].twin;
                    if (twin != std::numeric_limits<uint32_t>::max()) {
                        // Test to see if start location was inside this twin
                        sm::vec<sm::vec<float>, 3> tv_lf = this->triangle_vertices (twin, model_to_scene);
                        sm::vec<float> _tn = this->triangle_normal (tv_lf);
                        auto [is, h] = sm::geometry::ray_tri_intersection<float> (tv_lf[0], tv_lf[1], tv_lf[2], camloc_sf + (_tn / 2.0f), -_tn);
                        if constexpr (debug_move) { std::cout << "Start of move " << (is ? "IS" : "is NOT") << " in twin " << twin << std::endl; }
                        if (is) {
                            if constexpr (debug_move) { std::cout << "CORRECT ti0 to the twin " << twin << std::endl; }
                            // We're in this neighbour, so update ti0/tn0 and mark isect true
                            this->ti0 = twin;
                            tv_sf = tv_lf;
                            tn0 = _tn;
                            isect = true;
                            // This requires a number of matrix recomputations:
                            hov_sf = h;
                            cam_to_surface = cam_to_scene;
                            cam_to_surface.pretranslate (hov_sf - camloc_sf); // This is our init pose, placed on the surface
                            break; // out of do-while
                        }
                    }
                    hi = this->halfedge[hi].next;
                } while (hi != this->ti0);

                if (isect == false) {
                    if constexpr (debug_move2) { std::cout << "DBG No intersection (at start) with twins" << std::endl; }

                    // Final test to see if we're on boundary?
                    float closest_edge_d = sm::geometry::dist_to_tri_edge (tv_sf[0], tv_sf[1], tv_sf[2], camloc_sf - (tn0 * hoverheight));
                    if constexpr (debug_move2) {
                        std::cout << "Closest distance from " << (camloc_sf - (tn0 * hoverheight)) << " to ti0 edge: " << closest_edge_d << std::endl;
                    }
                    constexpr float ced_thresh = std::numeric_limits<float>::epsilon() * 50;
                    if (closest_edge_d < ced_thresh) {
                        // make tiny adjustment to camloc_sf so we ARE in the triangle? OR...
                        isect = true; // SAY we are, and proceed? <-- this if it works.
                    } else {
                        ne.m_type = NavException::type::no_intersection;
                        throw ne;
                    }
                } else {
                    if constexpr (debug_move2) {
                        std::cout << "Found intersection (at start) with twin triangle " << this->ti0 << std::endl;
                    }
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

            // New section to handle the case that we started right on a vertex
            if (isect == true && int_vertex_hi != std::numeric_limits<uint32_t>::max()) {
                // We HAVE a vertex intersection. Check if we either cross, or land in one of this vertex's neighbours to correct our starting triangle and normal.
                auto onens = this->find_neighbours (int_vertex_hi);
                for (auto _ti : onens) {
                    sm::vec<sm::vec<float>, 3> tv_nb = this->triangle_vertices (_ti, model_to_scene);
                    auto _tn = this->triangle_normal (tv_nb); // gets normal in *scene frame*
                    sm::vec<float> _mv_orthog = _tn * (mv_sf.dot (_tn) / (_tn.dot (_tn))); // This tn needs to be in scene frame
                    sm::vec<float> _mv_inplane = mv_sf - _mv_orthog; // scene frame, a relative movement
                    crossing_data cd = this->compute_crossing_location (tv_nb, _ti, hov_sf, _mv_inplane);
                    if (cd.pm.flags.test (pm_fl::no_cross_point) == false) {
                        this->ti0 = _ti;
                        tn0 = _tn;
                        tv_sf = tv_nb;
                        mv_orthog = _mv_orthog;
                        mv_inplane = _mv_inplane;
                        if constexpr (debug_move) { std::cout << "Break on cross point with triangle (" << _ti << ")\n"; }
                        break;
                    } else {
                        // No crossing, did we land in the triangle?
                        auto [is, h] = sm::geometry::ray_tri_intersection<float, double> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf + _mv_inplane + (_tn / 2.0f), -_tn);
                        if (is) { // then we DID land in this neighbour tri
                            this->ti0 = _ti;
                            tn0 = _tn;
                            tv_sf = tv_nb;
                            mv_orthog = _mv_orthog;
                            mv_inplane = _mv_inplane;
                            if constexpr (debug_move) { std::cout << "Break as we landed IN triangle (" << _ti << ")\n"; }
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
            uint32_t detected_edge = std::numeric_limits<uint32_t>::max();
            //sm::vec<float> detected_edgevec = {};
            uint32_t detected_newtri = std::numeric_limits<uint32_t>::max(); // new triangle detected as part of a vertex crossing

            // Now loop while our path may traverse one or more triangles
            while (!flags.test (cmm_fl::done)) {

                if constexpr (debug_move) {
                    std::cout << "\nWHILE LOOP\n"
                              << "ti0 = (" << this->ti0 << ")\n"
                              << "mv_inplane: " << hov_sf << "," << mv_inplane << "\n"
                              << "tn0 = " << tn0 << ")\n";
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
                crossing_data cd = this->compute_crossing_location (tv_sf, this->ti0, hov_sf, mv_inplane);

                if (cd.pm.flags.test (pm_fl::no_cross_point) == false || flags.test (cmm_fl::detected_crossing) || flags.test (cmm_fl::vertex_crossing)) {
                    // Then an edge (or vertex) crossing WAS detected (by compute_crossing_location or a prev. 'detected crossing')

                    if (flags.test (cmm_fl::detected_crossing)) {
                        if constexpr (debug_move) { std::cout << "This is a detected crossing; changing crossing_data.halfedge to " << detected_edge << std::endl; }
                        // We have to update our crossing data, as we detected a crossing over
                        // an edge (probably while moving along that edge)
                        cd.halfedge = detected_edge;
                        cd.tri_edge = this->edge_vector (detected_edge, model_to_scene);
                        cd.pm.mv = mv_inplane;
                        cd.pm.end = hov_sf + mv_inplane;
                    } else {
                        if constexpr (debug_move) { std::cout << "This IS a crossing (compute_crossing_location found it) " << std::endl; }
                    }

                    // _ti, _tn are the new triangle
                    sm::vec<float> _tn = {};
                    uint32_t _ti = std::numeric_limits<uint32_t>::max();
                    if (flags.test (cmm_fl::vertex_crossing)) {
                        if constexpr (debug_move) { std::cout << "Setting _ti to over-the-vertex tri " << detected_newtri << std::endl; }
                        _ti = detected_newtri;
                    } else {
                        // new triangle is the twin of the crossed edge
                        _ti = this->halfedge[cd.halfedge].twin;
                        if constexpr (debug_move) {
                            std::cout << "find triangle across edge: halfedge " << cd.halfedge << " gives the neighbour, which is its twin: " << _ti << std::endl;
                        }
                    }

                    if (_ti != std::numeric_limits<uint32_t>::max()) {

                        // Re-orient onto the new triangle
                        sm::vec<sm::vec<float>, 3> newtv_sf = this->triangle_vertices (_ti, model_to_scene);
                        _tn = this->triangle_normal (newtv_sf);

                        if constexpr (debug_move) { std::cout << "RE-ORIENT to _ti: " << _ti << " " << newtv_sf << " norm: " << _tn << "\n"; }

                        // If a vertex crossing, we have to make an edge that is the cross product of the two triangle normals
                        if (flags.test (cmm_fl::vertex_crossing)) { cd.tri_edge = tn0.cross (_tn); }

                        // Compute the reorientation due to the requested movement.
                        float rotn_angle = tn0.angle (_tn, cd.tri_edge);
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
                            auto [isect2, isectpoint2] = sm::geometry::ray_tri_intersection<float, float> (newtv_sf[0], newtv_sf[1], newtv_sf[2], endmv + (_tn / 2.0f), -_tn);
                            if constexpr (debug_move) { std::cout << "endmv = " << endmv << " DOES" << (isect2 ? "" : " NOT") << " land in new triangle\n"; }
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
                        tn0 = _tn;

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
                            std::cout << "No cross point and not colinear.\n  Testing if " << (hov_sf + mv_inplane + (tn0 / 2.0f)) << "," << -tn0
                                      << " intersects tv_sf (" << tv_sf << "\n";
                        }
                        auto [single_mv, he] = sm::geometry::ray_tri_intersection<float, float> (tv_sf[0], tv_sf[1], tv_sf[2], hov_sf + mv_inplane + (tn0 / 2.0f), -tn0);
                        flags.set (cmm_fl::single_movement, single_mv);
                    }

                    if (flags.test (cmm_fl::single_movement)) {
                        // Perform simplest movement, which is just to translate by mv_inplane
                        if constexpr (debug_move) { std::cout << "End of movement is *still* in ti0, so move mv_inplane\n"; }
                        cam_to_surface.pretranslate (mv_inplane);
                        flags.set (cmm_fl::done, true);

                    } else {
                        if constexpr (debug_move) { std::cout << "End of movement is NOT in ti0 " << this->ti0 << ". Look for start neighbours\n"; }

                        // Test neighbours, new scheme using halfedge data structures
                        // Test neighbours to find any for which the start location is also within-boundary
                        flags.set (cmm_fl::detected_crossing, false);
                        flags.set (cmm_fl::vertex_crossing, false);

                        uint32_t _ti_2n = std::numeric_limits<uint32_t>::max();
                        sm::vec<float>_tn_2n = {};
                        sm::vec<float> _tn = {};

                        // TWO NEIGHBOURS
                        std::set<uint32_t> neighbours_tested;
                        uint32_t hi = this->ti0;
                        do {
                            uint32_t twin = this->halfedge[hi].twin;
                            if (twin != std::numeric_limits<uint32_t>::max()) {
                                // Test to see if start location was inside a neighbour
                                sm::vec<sm::vec<float>, 3> tv_nb = this->triangle_vertices (twin, model_to_scene);
                                _tn = this->triangle_normal (tv_nb);

                                auto [is, h] = sm::geometry::ray_tri_intersection<float, double> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf + (_tn / 2.0f), -_tn);
                                sm::vec<float> mv_orthog_nb = _tn * (mv_inplane.dot (_tn) / (_tn.dot(_tn)));
                                sm::vec<float> mv_inplane_nb = mv_inplane - mv_orthog_nb;
                                if constexpr (debug_move) {
                                    std::cout << "TN: " << twin << ": isect vector " << (hov_sf + mv_inplane_nb + (_tn / 2.0f)) << "," << -_tn << " with tri " << tv_nb;
                                }
                                auto [endis, endh] = sm::geometry::ray_tri_intersection<float, double> (tv_nb[0], tv_nb[1], tv_nb[2], hov_sf + mv_inplane_nb + (_tn / 2.0f), -_tn);
                                if constexpr (debug_move) {
                                    std::cout << " Start IN? " << (is ? "Y" : "N") << "End IN? " << (endis ? "Y" : "N") << std::endl;
                                }

                                neighbours_tested.insert (twin);

                                // Here, start is in original, end may not be in original. This is an 'intersection detected crossing' of a triangle edge
                                // which wasn't picked up with compute_crossing_location
                                if (endis) {
                                    // End is in neighbour so this is a detected crossing
                                    if constexpr (debug_move) { std::cout << "DETECTED crossing! Pass on to next loop!\n"; }
                                    flags.set (cmm_fl::detected_crossing, true);
                                    detected_edge = hi;
                                    //detected_edgevec = tv_nb[i2] - tv_nb[i1]; // no longer need as we have detected_edge, a halfedge? Can construct later?
                                    break; // out of for
                                } else { // end not in neighbour
                                    if (is) { // start is in neighbour tri (will re-orient to this and re-loop)
                                        _ti_2n = twin;
                                        _tn_2n = _tn;
                                        break; // out of for
                                    } // Neither end nor start are in neighbour. This occurs if the end is ON the boundary, but precision errors
                                      // mean this location isn't 'in' either start or neighbour (according to ray_tri_intersection)
                                }
                            }

                            hi = this->halfedge[hi].next;

                        } while (hi != this->ti0);

                        // Test one-neighbours here if necessary (that is, if the two neighbour test above failed)
                        if (flags.test (cmm_fl::detected_crossing) == false && _ti_2n == std::numeric_limits<uint32_t>::max()) {
                            uint32_t hi = this->ti0;
                            do { // For each half edge around ti0
                                std::vector<uint32_t> nbs = this->find_neighbours (hi);
                                for (auto _ti : nbs) {
                                    // Already tested? This should avoid us testing any two-neighbours here (already did them above)
                                    if (neighbours_tested.count (_ti)) { continue; }
                                    neighbours_tested.insert (_ti);

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
                                        std::cout << "Start of move " << (is ? "IS" : "is NOT") << " in ONE-neighbour " << _ti << " / " <<  tv_nb << std::endl;
                                        std::cout << "And End of move " << (endis ? "IS" : "is NOT") << " in that ONE-neighbour " << std::endl;
                                    }

                                    if (endis) {
                                        // End is in one-neighbour so this is a detected crossing
                                        if constexpr (debug_move) { std::cout << "DETECTED crossing over ONE-neighbour! Pass on to next loop!\n"; }
                                        flags.set (cmm_fl::vertex_crossing, true);
                                        detected_edge = hi;
                                        //detected_edgevec = {std::numeric_limits<float>::max()}; // to be the cross product of the last-triangle normal and the newtri normal.
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
                                hi = this->halfedge[hi].next;

                            } while (hi != this->ti0);
                        }

                        if (_ti_2n != std::numeric_limits<uint32_t>::max()) {
                            // Now we know an alternative start triangle for the movement. Re-orient to this and re-loop
                            this->ti0 = _ti_2n;
                            tn0 = _tn;
                            // recompute mv_inplane for this neighbour triangle
                            mv_orthog = tn0 * (mv_sf.dot (tn0) / (tn0.dot (tn0)));
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
            cam_to_surface.pretranslate (hoverheight * tn0);
            if constexpr (debug_move) {
                std::cout << "looping mv_inplanes completed. Final camloc_sf: " << cam_to_surface.translation() << std::endl;
            }
            return cam_to_surface;

        } // compute_mesh_movement

    }; // struct NavMesh

} // namespace
