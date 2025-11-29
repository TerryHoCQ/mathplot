// This version of jc_voronoi.h has been modified by Seb James. jcv_point has been
// changed (from a struct of two jcv_reals to a mplot::vec<jcv_real, 3> allowing the
// formation of a '2.5D' Voronoi surface)

// Copyright (c) 2015-2023 Mathias Westerdahl
// For LICENSE (MIT), USAGE or HISTORY, see bottom of file

#pragma once

#include <stdlib.h> // malloc() & free()
#include <limits>
#include <cmath>
#include <cassert>  // assert()
#include <cstdint>  // uintptr_t etc
#include <functional>
#include <sm/mathconst>
#include <sm/vec>

#ifndef JCV_EDGE_INTERSECT_THRESHOLD
    // Fix for Issue #40
    #define JCV_EDGE_INTERSECT_THRESHOLD 1.0e-10F
#endif

namespace jc
{
#pragma pack(push, 1)

    template<typename T>
    using jcv_point = sm::vec<T, 3>;

    // fwd declar graphpedge
    template <typename T> struct jcv_graphedge;

    template<typename T>
    struct jcv_site
    {
        jcv_point<T>       p;
        int                       index;  // Index into the original list of points
        jcv_graphedge<T>*  edges;  // The half edges owned by the cell
    };

    // The coefficients a, b and c are from the general line equation: ax * by + c = 0
    template<typename T>
    struct edge
    {
        struct edge<T>*   next;
        jcv_site<T>*          sites[2];
        jcv_point<T>          pos[2];
        T            a;
        T            b;
        T            c;
    };

    template<typename T>
    struct jcv_graphedge
    {
        struct jcv_graphedge<T>*  next;
        struct edge<T>*       edge_;
        struct jcv_site<T>*       neighbor;
        jcv_point<T>              pos[2];
        T                         angle;
    };

    template<typename T>
    struct jcv_delauney_iter
    {
        const edge<T>*   sentinel;
        const edge<T>*   current;
    };

    template<typename T>
    struct jcv_delauney_edge
    {
        const edge<T>*     edge_;      // The voronoi edge separating the two sites
        const jcv_site<T>* sites[2];
        jcv_point<T>       pos[2];     // the positions of the two sites
    };

    template<typename T>
    struct jcv_rect
    {
        jcv_point<T>   min;
        jcv_point<T>   max;
    };

    // Convert these to templated usings? or use std::function?
    /// Tests if a point is inside the final shape
    //typedef int (*jcv_clip_test_point_fn)(const jcv_clipper<T>* clipper, const jcv_point p);
    //template<typename T>

    /** Given an edge, and the clipper, calculates the e->pos[0] and e->pos[1]
     * Returns 0 if not successful
     */
    //typedef int (*jcv_clip_edge_fn)(const jcv_clipper* clipper, edge* e);

    /** Given the clipper, the site and the last edge,
     * closes any gaps in the polygon by adding new edges that follow the bounding shape
     * The internal context is use when allocating new edges.
     */
    //typedef void (*jcv_clip_fillgap_fn)(const jcv_clipper* clipper, jcv_context_internal* allocator, jcv_site* s);

    // Forward declare jcv_context_internal for the std::function
    template<typename T> struct jcv_context_internal;

    template<typename T>
    struct jcv_clipper
    {
        // Tests if a point is inside the final shape
        std::function<int(const jcv_clipper<T>* clipper, const jcv_point<T> p)> test_fn;
        // Given an edge, and the clipper, calculates the e->pos[0] and e->pos[1]
        // Returns 0 if not successful
        std::function<int(const jcv_clipper<T>* clipper, edge<T>* e)> clip_fn;
        // Given the clipper, the site and the last edge,
        // closes any gaps in the polygon by adding new edges that follow the bounding shape
        // The internal context is use when allocating new edges.
        std::function<void(const jcv_clipper<T>* clipper, jcv_context_internal<T>* allocator, jcv_site<T>* s)> fill_fn;
        jcv_point<T>     min;        // The bounding rect min
        jcv_point<T>     max;        // The bounding rect max
        void*                   ctx;        // User defined context
    };

    // Second batch of structs
    template<typename T>
    struct jcv_halfedge
    {
        edge<T>*                edge_;
        struct jcv_halfedge<T>* left;
        struct jcv_halfedge<T>* right;
        jcv_point<T>            vertex;
        T                       y;
        int                     direction; // 0=left, 1=right
        int                     pqpos;
    };

    struct jcv_memoryblock
    {
        size_t sizefree;
        struct jcv_memoryblock* next;
        char*  memory;
    };

    typedef int  (*FJCVPriorityQueuePrint)(const void* node, int pos);

    struct jcv_priorityqueue
    {
        // Implements a binary heap
        int                         maxnumitems;
        int                         numitems;
        void**                      items;
    };

    using FJCVAllocFn = void*(void* userctx, size_t size);
    using FJCVFreeFn = void(void* userctx, void* p);

    template<typename T>
    struct jcv_context_internal
    {
        void*                         mem;
        edge<T>*           edges;
        jcv_halfedge<T>*       beachline_start;
        jcv_halfedge<T>*       beachline_end;
        jcv_halfedge<T>*       last_inserted;
        jcv_priorityqueue*            eventqueue;

        jcv_site<T>*           sites;
        jcv_site<T>*           bottomsite;
        int                           numsites;
        int                           currentsite;
        int                           _padding;

        jcv_memoryblock*              memblocks;
        edge<T>*           edgepool;
        jcv_halfedge<T>*       halfedgepool;
        void**                        eventmem;
        jcv_clipper<T>         clipper;

        void*                         memctx; // Given by the user
        std::function<FJCVAllocFn>    alloc;
        std::function<FJCVFreeFn>     free;

        jcv_rect<T>            rect;
    };

    template<typename T>
    struct jcv_diagram
    {
        jcv_context_internal<T>*   internal;
        int                               numsites;
        jcv_point<T>               min;
        jcv_point<T>               max;
    };

#pragma pack(pop)

    // The mananger class. Type T is what is called jcv_real in the original code
    template<typename T> requires std::is_floating_point_v<T>
    struct manager
    {
        manager(){}
        ~manager()
        {
            if (this->diagram) {
                jc::manager<T>::jcv_diagram_free (this->diagram);
                delete this->diagram;
            }
        }

        static constexpr T edge_intersect_threshold = T{JCV_EDGE_INTERSECT_THRESHOLD};

        // INTERNAL FUNCTIONS

        static const int JCV_DIRECTION_LEFT  = 0;
        static const int JCV_DIRECTION_RIGHT = 1;

        static constexpr T jcv_invalid_value = std::numeric_limits<T>::lowest();

        // App specific equality
        static int T_eq(T a, T b)
        {
            return std::abs(a - b) < std::numeric_limits<T>::epsilon();
        }

        // jcv_point
        static int jcv_point_cmp(const void* p1, const void* p2)
        {
            const jcv_point<T>* s1 = static_cast<const jcv_point<T>*>(p1);
            const jcv_point<T>* s2 = static_cast<const jcv_point<T>*>(p2);
            return (s1->y() != s2->y()) ? (s1->y() < s2->y() ? -1 : 1) : (s1->x() < s2->x() ? -1 : 1);
        }

        static int jcv_point_less (const jcv_point<T>* pt1, const jcv_point<T>* pt2 )
        {
            return (pt1->y() == pt2->y()) ? (pt1->x() < pt2->x()) : pt1->y() < pt2->y();
        }

        static int jcv_point_eq( const jcv_point<T>* pt1, const jcv_point<T>* pt2 )
        {
            return T_eq(pt1->y(), pt2->y()) && T_eq(pt1->x(), pt2->x());
        }

        [[maybe_unused]]
        static int jcv_point_on_box_edge( const jcv_point<T>* pt, const jcv_point<T>* min, const jcv_point<T>* max )
        {
            return pt->x() == min->x() || pt->y() == min->y() || pt->x() == max->x() || pt->y() == max->y();
        }

        // edges and corners
        static const int EDGE_LEFT    = 1;
        static const int EDGE_RIGHT   = 2;
        static const int EDGE_BOTTOM  = 4;
        static const int EDGE_TOP     = 8;

        static const int JCV_CORNER_NONE          = 0;
        static const int JCV_CORNER_TOP_LEFT      = 1;
        static const int JCV_CORNER_BOTTOM_LEFT   = 2;
        static const int JCV_CORNER_BOTTOM_RIGHT  = 3;
        static const int JCV_CORNER_TOP_RIGHT     = 4;

        static int jcv_get_edge_flags( const jcv_point<T>* pt, const jcv_point<T>* min, const jcv_point<T>* max )
        {
            int flags = 0;
            if      (pt->x() == min->x())   flags |= EDGE_LEFT;
            else if (pt->x() == max->x())   flags |= EDGE_RIGHT;
            if      (pt->y() == min->y())   flags |= EDGE_BOTTOM;
            else if (pt->y() == max->y())   flags |= EDGE_TOP;
            return flags;
        }

        static int edge_flags_to_corner(int edge_flags)
        {
#define TEST_FLAGS(_FLAGS, _RETVAL) if ( (_FLAGS) == edge_flags ) return _RETVAL
            TEST_FLAGS(EDGE_TOP|EDGE_LEFT, JCV_CORNER_TOP_LEFT);
            TEST_FLAGS(EDGE_TOP|EDGE_RIGHT, JCV_CORNER_TOP_RIGHT);
            TEST_FLAGS(EDGE_BOTTOM|EDGE_LEFT, JCV_CORNER_BOTTOM_LEFT);
            TEST_FLAGS(EDGE_BOTTOM|EDGE_RIGHT, JCV_CORNER_BOTTOM_RIGHT);
#undef TEST_FLAGS
            return 0;
        }

        [[maybe_unused]] static int jcv_is_corner(int corner) { return corner != 0; }

        static int jcv_corner_rotate_90(int corner)
        {
            corner--;
            corner = (corner+1)%4;
            return corner + 1;
        }
        static jcv_point<T> jcv_corner_to_point(int corner, const jcv_point<T>* min, const jcv_point<T>* max )
        {
            jcv_point<T> p;
            if      (corner == JCV_CORNER_TOP_LEFT)     { p[0] = min->x(); p[1] = max->y(); }
            else if (corner == JCV_CORNER_TOP_RIGHT)    { p[0] = max->x(); p[1] = max->y(); }
            else if (corner == JCV_CORNER_BOTTOM_LEFT)  { p[0] = min->x(); p[1] = min->y(); }
            else if (corner == JCV_CORNER_BOTTOM_RIGHT) { p[0] = max->x(); p[1] = min->y(); }
            else                                        { p[0] = jcv_invalid_value; p[1] = jcv_invalid_value; }
            return p;
        }

        static T jcv_point_dist_sq( const jcv_point<T>* pt1, const jcv_point<T>* pt2)
        {
            T diffx = pt1->x() - pt2->x();
            T diffy = pt1->y() - pt2->y();
            return diffx * diffx + diffy * diffy;
        }

        static T jcv_point_dist( const jcv_point<T>* pt1, const jcv_point<T>* pt2 )
        {
            return std::sqrt (jcv_point_dist_sq (pt1, pt2));
        }

        // Uses free (or the registered custom free function)
        static void jcv_diagram_free( jcv_diagram<T>* d )
        {
            jcv_context_internal<T>* internal = d->internal;
            void* memctx = internal->memctx;
            while(internal->memblocks)
            {
                jcv_memoryblock* p = internal->memblocks;
                internal->memblocks = internal->memblocks->next;
                internal->free( memctx, p );
            }

            internal->free( memctx, internal->mem );
        }

        // Returns an array of sites, where each index is the same as the original input point array.
        static const jcv_site<T>* jcv_diagram_get_sites( const jcv_diagram<T>* diagram )
        {
            return diagram->internal->sites;
        }

        // User API
        const jcv_site<T>* diagram_get_sites()
        {
            const jcv_site<T>* sites = nullptr;
            if (this->diagram) {
                sites = jcv_diagram_get_sites (this->diagram);
            }
            return sites;
        }

        // Iterates over a list of edges, skipping invalid edges (where p0==p1)
        const edge<T>* jcv_diagram_get_next_edge( const edge<T>* _edge )
        {
            const edge<T>* e = _edge->next;
            while (e != 0 && jcv_point_eq(&e->pos[0], &e->pos[1])) {
                e = e->next;
            }
            return e;
        }

        // Returns a linked list of all the voronoi edges excluding the ones that lie on the borders of
        // the bounding box.  For a full list of edges, you need to iterate over the sites, and their
        // graph edges.
        const edge<T>* jcv_diagram_get_edges( const jcv_diagram<T>* diagram )
        {
            edge<T> e;
            e.next = diagram->internal->edges;
            return jcv_diagram_get_next_edge(&e);
        }

        // Creates an iterator over the delauney edges of a voronoi diagram
        void jcv_delauney_begin( const jcv_diagram<T>* diagram, jcv_delauney_iter<T>* iter )
        {
            iter->current = 0;
            iter->sentinel = jcv_diagram_get_edges(diagram);
        }

        // Steps the iterator and returns the next edge Returns 0 when there are no more edges
        int jcv_delauney_next( jcv_delauney_iter<T>* iter, jcv_delauney_edge<T>* next )
        {
            if (iter->sentinel)
            {
                iter->current = iter->sentinel;
                iter->sentinel = 0;
            }
            else {
                // Note: If we use the raw edges, we still get a proper delauney triangulation
                // However, the result looks less relevant to the cells contained within the bounding box
                // E.g. some cells that look isolated from each other, suddenly still are connected,
                // because they share an edge outside of the bounding box
                iter->current = jcv_diagram_get_next_edge(iter->current);
            }

            while (iter->current && (iter->current->sites[0] == 0 || iter->current->sites[1] == 0))
            {
                iter->current = jcv_diagram_get_next_edge(iter->current);
            }

            if (!iter->current)
                return 0;

            next->edge_ = iter->current;
            next->sites[0] = next->edge_->sites[0];
            next->sites[1] = next->edge_->sites[1];
            next->pos[0] = next->sites[0]->p;
            next->pos[1] = next->sites[1]->p;
            return 1;
        }

        static void* jcv_align(void* value, size_t alignment)
        {
            return (void*) (((uintptr_t) value + (alignment-1)) & ~(alignment-1));
        }

        static void* jcv_alloc(jcv_context_internal<T>* internal, size_t size)
        {
            if( !internal->memblocks || internal->memblocks->sizefree < (size+sizeof(void*)) )
            {
                size_t blocksize = 16 * 1024;
                jcv_memoryblock* block = (jcv_memoryblock*)internal->alloc( internal->memctx, blocksize );
                size_t offset = sizeof(jcv_memoryblock);
                block->sizefree = blocksize - offset;
                block->next = internal->memblocks;
                block->memory = ((char*)block) + offset;
                internal->memblocks = block;
            }
            void* p_raw = internal->memblocks->memory;
            void* p_aligned = jcv_align(p_raw, sizeof(void*));
            size += (uintptr_t)p_aligned - (uintptr_t)p_raw;
            internal->memblocks->memory += size;
            internal->memblocks->sizefree -= size;
            return p_aligned;
        }

        static edge<T>* jcv_alloc_edge(jcv_context_internal<T>* internal)
        {
            return (edge<T>*)jcv_alloc(internal, sizeof(edge<T>));
        }

        static jcv_halfedge<T>* jcv_alloc_halfedge(jcv_context_internal<T>* internal)
        {
            if( internal->halfedgepool )
            {
                jcv_halfedge<T>* edge = internal->halfedgepool;
                internal->halfedgepool = internal->halfedgepool->right;
                return edge;
            }

            return (jcv_halfedge<T>*)jcv_alloc(internal, sizeof(jcv_halfedge<T>));
        }

        static jcv_graphedge<T>* jcv_alloc_graphedge(jcv_context_internal<T>* internal)
        {
            return (jcv_graphedge<T>*)jcv_alloc(internal, sizeof(jcv_graphedge<T>));
        }

        static void* jcv_alloc_fn(void* memctx, size_t size)
        {
            (void)memctx;
            return malloc(size);
        }

        static void jcv_free_fn(void* memctx, void* p)
        {
            (void)memctx;
            free(p);
        }

        // edge methods
        static int jcv_is_valid(const jcv_point<T>* p)
        {
            return (p->x() != jcv_invalid_value || p->y() != jcv_invalid_value) ? 1 : 0;
        }

        static void edge_create(edge<T>* e, jcv_site<T>* s1, jcv_site<T>* s2)
        {
            e->next = 0;
            e->sites[0] = s1;
            e->sites[1] = s2;
            e->pos[0][0] = jcv_invalid_value;
            e->pos[0][1] = jcv_invalid_value;
            e->pos[1][0] = jcv_invalid_value;
            e->pos[1][1] = jcv_invalid_value;

            // Create line equation between S1 and S2:
            // T a = -1 * (s2->p[1] - s1->p[1]);
            // T b = s2->p[0] - s1->p[0];
            // //T c = -1 * (s2->p[0] - s1->p[0]) * s1->p[1] + (s2->p[1] - s1->p[1]) * s1->p[0];
            //
            // // create perpendicular line
            // T pa = b;
            // T pb = -a;
            // //T pc = pa * s1->p[0] + pb * s1->p[1];
            //
            // // Move to the mid point
            // T mx = s1->p[0] + dx * T(0.5);
            // T my = s1->p[1] + dy * T(0.5);
            // T pc = ( pa * mx + pb * my );

            T dx = s2->p[0] - s1->p[0];
            T dy = s2->p[1] - s1->p[1];
            int dx_is_larger = (dx*dx) > (dy*dy); // instead of fabs

            // Simplify it, using dx and dy
            e->c = dx * (s1->p[0] + dx * (T)0.5) + dy * (s1->p[1] + dy * (T)0.5);

            if( dx_is_larger )
            {
                e->a = (T)1;
                e->b = dy / dx;
                e->c /= dx;
            }
            else
            {
                e->a = dx / dy;
                e->b = (T)1;
                e->c /= dy;
            }
        }

        // CLIPPING
        static int jcv_boxshape_test(const jcv_clipper<T>* clipper, const jcv_point<T> p)
        {
            return p[0] >= clipper->min[0] && p[0] <= clipper->max[0] &&
            p[1] >= clipper->min[1] && p[1] <= clipper->max[1];
        }

        // The line equation: ax + by + c = 0
        // see edge_create
        static int jcv_boxshape_clip(const jcv_clipper<T>* clipper, edge<T>* e)
        {
            T pxmin = clipper->min[0];
            T pxmax = clipper->max[0];
            T pymin = clipper->min[1];
            T pymax = clipper->max[1];

            T x1, y1, x2, y2;
            jcv_point<T>* s1;
            jcv_point<T>* s2;
            if (e->a == (T)1 && e->b >= (T)0)
            {
                s1 = jcv_is_valid(&e->pos[1]) ? &e->pos[1] : 0;
                s2 = jcv_is_valid(&e->pos[0]) ? &e->pos[0] : 0;
            }
            else
            {
                s1 = jcv_is_valid(&e->pos[0]) ? &e->pos[0] : 0;
                s2 = jcv_is_valid(&e->pos[1]) ? &e->pos[1] : 0;
            }

            if (e->a == (T)1) // delta x is larger
            {
                y1 = pymin;
                if (s1 != 0 && s1->y() > pymin)
                {
                    y1 = s1->y();
                }
                if( y1 > pymax )
                {
                    y1 = pymax;
                }
                x1 = e->c - e->b * y1;
                y2 = pymax;
                if (s2 != 0 && s2->y() < pymax)
                    y2 = s2->y();

                if( y2 < pymin )
                {
                    y2 = pymin;
                }
                x2 = (e->c) - (e->b) * y2;
                // Never occurs according to lcov
                // if( ((x1 > pxmax) & (x2 > pxmax)) | ((x1 < pxmin) & (x2 < pxmin)) )
                // {
                //     return 0;
                // }
                if (x1 > pxmax)
                {
                    x1 = pxmax;
                    y1 = (e->c - x1) / e->b;
                }
                else if (x1 < pxmin)
                {
                    x1 = pxmin;
                    y1 = (e->c - x1) / e->b;
                }
                if (x2 > pxmax)
                {
                    x2 = pxmax;
                    y2 = (e->c - x2) / e->b;
                }
                else if (x2 < pxmin)
                {
                    x2 = pxmin;
                    y2 = (e->c - x2) / e->b;
                }
            }
            else // delta y is larger
            {
                x1 = pxmin;
                if( s1 != 0 && s1->x() > pxmin )
                    x1 = s1->x();
                if( x1 > pxmax )
                {
                    x1 = pxmax;
                }
                y1 = e->c - e->a * x1;
                x2 = pxmax;
                if( s2 != 0 && s2->x() < pxmax )
                    x2 = s2->x();
                if( x2 < pxmin )
                {
                    x2 = pxmin;
                }
                y2 = e->c - e->a * x2;
                // Never occurs according to lcov
                // if( ((y1 > pymax) & (y2 > pymax)) | ((y1 < pymin) & (y2 < pymin)) )
                // {
                //     return 0;
                // }
                if( y1 > pymax )
                {
                    y1 = pymax;
                    x1 = (e->c - y1) / e->a;
                }
                else if( y1 < pymin )
                {
                    y1 = pymin;
                    x1 = (e->c - y1) / e->a;
                }
                if( y2 > pymax )
                {
                    y2 = pymax;
                    x2 = (e->c - y2) / e->a;
                }
                else if( y2 < pymin )
                {
                    y2 = pymin;
                    x2 = (e->c - y2) / e->a;
                }
            }

            e->pos[0][0] = x1;
            e->pos[0][1] = y1;
            e->pos[1][0] = x2;
            e->pos[1][1] = y2;

            // If the two points are equal, the result is invalid
            return (x1 == x2 && y1 == y2) ? 0 : 1;
        }

        // The line equation: ax + by + c = 0
        // see edge_create
        static int edge_clipline(jcv_context_internal<T>* internal, edge<T>* e)
        {
            return internal->clipper.clip_fn(&internal->clipper, e);
        }

        static edge<T>* edge_new(jcv_context_internal<T>* internal, jcv_site<T>* s1, jcv_site<T>* s2)
        {
            edge<T>* e = jcv_alloc_edge(internal);
            edge_create(e, s1, s2);
            return e;
        }


        // jcv_halfedge

        static void jcv_halfedge_link(jcv_halfedge<T>* edge, jcv_halfedge<T>* newedge)
        {
            newedge->left = edge;
            newedge->right = edge->right;
            edge->right->left = newedge;
            edge->right = newedge;
        }

        static void jcv_halfedge_unlink(jcv_halfedge<T>* he)
        {
            he->left->right = he->right;
            he->right->left = he->left;
            he->left  = 0;
            he->right = 0;
        }

        static jcv_halfedge<T>* jcv_halfedge_new(jcv_context_internal<T>* internal, edge<T>* e, int direction)
        {
            jcv_halfedge<T>* he = jcv_alloc_halfedge(internal);
            he->edge_       = e;
            he->left        = 0;
            he->right       = 0;
            he->direction   = direction;
            he->pqpos       = 0;
            // These are set outside
            //he->y()
            //he->vertex
            return he;
        }

        static void jcv_halfedge_delete(jcv_context_internal<T>* internal, jcv_halfedge<T>* he)
        {
            he->right = internal->halfedgepool;
            internal->halfedgepool = he;
        }

        static jcv_site<T>* jcv_halfedge_leftsite(const jcv_halfedge<T>* he)
        {
            return he->edge_->sites[he->direction];
        }

        static jcv_site<T>* jcv_halfedge_rightsite(const jcv_halfedge<T>* he)
        {
            return he->edge_ ? he->edge_->sites[1 - he->direction] : 0;
        }

        static int jcv_halfedge_rightof(const jcv_halfedge<T>* he, const jcv_point<T>* p)
        {
            const edge<T>* e = he->edge_;
            const jcv_site<T>* topsite = e->sites[1];

            int right_of_site = (p->x() > topsite->p[0]) ? 1 : 0;
            if (right_of_site && he->direction == JCV_DIRECTION_LEFT)
                return 1;
            if (!right_of_site && he->direction == JCV_DIRECTION_RIGHT)
                return 0;

            T dxp, dyp, dxs, t1, t2, t3, yl;

            int above;
            if (e->a == (T)1)
            {
                dyp = p->y() - topsite->p[1];
                dxp = p->x() - topsite->p[0];
                int fast = 0;
                if( (!right_of_site & (e->b < (T)0)) | (right_of_site & (e->b >= (T)0)) )
                {
                    above = dyp >= e->b * dxp;
                    fast = above;
                }
                else
                {
                    above = (p->x() + p->y() * e->b) > e->c;
                    if (e->b < (T)0)
                        above = !above;
                    if (!above)
                        fast = 1;
                }
                if (!fast)
                {
                    dxs = topsite->p[0] - e->sites[0]->p[0];
                    above = e->b * (dxp * dxp - dyp * dyp)
                    < dxs * dyp * ((T)1 + (T)2 * dxp / dxs + e->b * e->b);
                    if (e->b < (T)0)
                        above = !above;
                }
            }
            else // e->b == 1
            {
                yl = e->c - e->a * p->x();
                t1 = p->y() - yl;
                t2 = p->x() - topsite->p[0];
                t3 = yl - topsite->p[1];
                above = t1 * t1 > (t2 * t2 + t3 * t3);
            }
            return (he->direction == JCV_DIRECTION_LEFT ? above : !above);
        }

        // Keeps the priority queue sorted with events sorted in ascending order
        // Return 1 if the edges needs to be swapped
        static int jcv_halfedge_compare( const jcv_halfedge<T>* he1, const jcv_halfedge<T>* he2 )
        {
            return  (he1->y == he2->y) ? he1->vertex[0] > he2->vertex[0] : he1->y > he2->y;
        }

        static int jcv_halfedge_intersect(const jcv_halfedge<T>* he1, const jcv_halfedge<T>* he2, jcv_point<T>* out)
        {
            const edge<T>* e1 = he1->edge_;
            const edge<T>* e2 = he2->edge_;

            T d = e1->a * e2->b - e1->b * e2->a;
            if(-edge_intersect_threshold < d && d < edge_intersect_threshold)
            {
                return 0;
            }
            (*out)[0] = (e1->c * e2->b - e1->b * e2->c) / d;
            (*out)[1] = (e1->a * e2->c - e1->c * e2->a) / d;
            // I considered trying to determine the correct z here, but we don't have all the
            // information required. So just set out->z to a default value meaning 'unset' (Seb)
            (*out)[2] = 0.0f; // NB: this does not set z for all edges

            const edge<T>* e;
            const jcv_halfedge<T>* he;
            if( jcv_point_less( &e1->sites[1]->p, &e2->sites[1]->p) )
            {
                he = he1;
                e = e1;
            }
            else
            {
                he = he2;
                e = e2;
            }

            int right_of_site = out->x() >= e->sites[1]->p[0];
            if ((right_of_site && he->direction == JCV_DIRECTION_LEFT) || (!right_of_site && he->direction == JCV_DIRECTION_RIGHT))
            {
                return 0;
            }

            return 1;
        }


        // Priority queue

        static int jcv_pq_moveup(jcv_priorityqueue* pq, int pos)
        {
            jcv_halfedge<T>** items = (jcv_halfedge<T>**)pq->items;
            jcv_halfedge<T>* node = items[pos];

            for( int parent = (pos >> 1);
                 pos > 1 && jcv_halfedge_compare(items[parent], node);
                 pos = parent, parent = parent >> 1)
            {
                items[pos] = items[parent];
                items[pos]->pqpos = pos;
            }

            node->pqpos = pos;
            items[pos] = node;
            return pos;
        }

        static int jcv_pq_maxchild(jcv_priorityqueue* pq, int pos)
        {
            int child = pos << 1;
            if( child >= pq->numitems )
                return 0;
            jcv_halfedge<T>** items = (jcv_halfedge<T>**)pq->items;
            if( (child + 1) < pq->numitems && jcv_halfedge_compare(items[child], items[child+1]) )
                return child+1;
            return child;
        }

        static int jcv_pq_movedown(jcv_priorityqueue* pq, int pos)
        {
            jcv_halfedge<T>** items = (jcv_halfedge<T>**)pq->items;
            jcv_halfedge<T>* node = items[pos];

            int child = jcv_pq_maxchild(pq, pos);
            while( child && jcv_halfedge_compare(node, items[child]) )
            {
                items[pos] = items[child];
                items[pos]->pqpos = pos;
                pos = child;
                child = jcv_pq_maxchild(pq, pos);
            }

            items[pos] = node;
            items[pos]->pqpos = pos;
            return pos;
        }

        static void jcv_pq_create(jcv_priorityqueue* pq, int capacity, void** buffer)
        {
            pq->maxnumitems = capacity;
            pq->numitems    = 1;
            pq->items       = buffer;
        }

        static int jcv_pq_empty(jcv_priorityqueue* pq)
        {
            return pq->numitems == 1 ? 1 : 0;
        }

        static int jcv_pq_push(jcv_priorityqueue* pq, void* node)
        {
            assert(pq->numitems < pq->maxnumitems);
            int n = pq->numitems++;
            pq->items[n] = node;
            return jcv_pq_moveup(pq, n);
        }

        static void* jcv_pq_pop(jcv_priorityqueue* pq)
        {
            void* node = pq->items[1];
            pq->items[1] = pq->items[--pq->numitems];
            jcv_pq_movedown(pq, 1);
            return node;
        }

        static void* jcv_pq_top(jcv_priorityqueue* pq)
        {
            return pq->items[1];
        }

        static void jcv_pq_remove(jcv_priorityqueue* pq, jcv_halfedge<T>* node)
        {
            if( pq->numitems == 1 )
                return;
            int pos = node->pqpos;
            if( pos == 0 )
                return;

            jcv_halfedge<T>** items = (jcv_halfedge<T>**)pq->items;

            items[pos] = items[--pq->numitems];
            if( jcv_halfedge_compare( node, items[pos] ) )
                jcv_pq_moveup( pq, pos );
            else
                jcv_pq_movedown( pq, pos );
            node->pqpos = pos;
        }

        // internal functions

        static jcv_site<T>* jcv_nextsite(jcv_context_internal<T>* internal)
        {
            return (internal->currentsite < internal->numsites) ? &internal->sites[internal->currentsite++] : 0;
        }

        static jcv_halfedge<T>* jcv_get_edge_above_x(jcv_context_internal<T>* internal, const jcv_point<T>* p)
        {
            // Gets the arc on the beach line at the x coordinate (i.e. right above the new site event)

            // A good guess it's close by (Can be optimized)
            jcv_halfedge<T>* he = internal->last_inserted;
            if( !he )
            {
                if( p->x() < (internal->rect.max[0] - internal->rect.min[0]) / 2 )
                    he = internal->beachline_start;
                else
                    he = internal->beachline_end;
            }

            //
            if( he == internal->beachline_start || (he != internal->beachline_end && jcv_halfedge_rightof(he, p)) )
            {
                do {
                    he = he->right;
                }
                while( he != internal->beachline_end && jcv_halfedge_rightof(he, p) );

                he = he->left;
            }
            else
            {
                do {
                    he = he->left;
                }
                while( he != internal->beachline_start && !jcv_halfedge_rightof(he, p) );
            }

            return he;
        }

        static int jcv_check_circle_event(const jcv_halfedge<T>* he1, const jcv_halfedge<T>* he2, jcv_point<T>* vertex)
        {
            edge<T>* e1 = he1->edge_;
            edge<T>* e2 = he2->edge_;
            if( e1 == 0 || e2 == 0 || e1->sites[1] == e2->sites[1] )
            {
                return 0;
            }

            return jcv_halfedge_intersect(he1, he2, vertex);
        }

        static void jcv_site_event(jcv_context_internal<T>* internal, jcv_site<T>* site)
        {
            jcv_halfedge<T>* left   = jcv_get_edge_above_x(internal, &site->p);
            jcv_halfedge<T>* right  = left->right;
            jcv_site<T>*     bottom = jcv_halfedge_rightsite(left);
            if( !bottom )
                bottom = internal->bottomsite;

            edge<T>* edge = edge_new(internal, bottom, site);
            edge->next = internal->edges;
            internal->edges = edge;

            jcv_halfedge<T>* edge1 = jcv_halfedge_new(internal, edge, JCV_DIRECTION_LEFT);
            jcv_halfedge<T>* edge2 = jcv_halfedge_new(internal, edge, JCV_DIRECTION_RIGHT);

            jcv_halfedge_link(left, edge1);
            jcv_halfedge_link(edge1, edge2);

            internal->last_inserted = right;

            jcv_point<T> p;
            if( jcv_check_circle_event( left, edge1, &p ) )
            {
                jcv_pq_remove(internal->eventqueue, left);
                left->vertex    = p;
                left->y         = p[1] + jcv_point_dist(&site->p, &p);
                jcv_pq_push(internal->eventqueue, left);
            }
            if( jcv_check_circle_event( edge2, right, &p ) )
            {
                edge2->vertex   = p;
                edge2->y        = p[1] + jcv_point_dist(&site->p, &p);
                jcv_pq_push(internal->eventqueue, edge2);
            }
        }

        // https://cp-algorithms.com/geometry/oriented-triangle-area.html
        static T jcv_determinant(const jcv_point<T>* a, const jcv_point<T>* b, const jcv_point<T>* c)
        {
            return (b->x() - a->x())*(c->y() - a->y()) - (b->y() - a->y())*(c->x() - a->x());
        }

        static T jcv_calc_sort_metric(const jcv_site<T>* site, const jcv_graphedge<T>* edge)
        {
            // We take the average of the two points, since we can better distinguish between very small edges
            constexpr T half = T{0.5};
            T x = (edge->pos[0][0] + edge->pos[1][0]) * half;
            T y = (edge->pos[0][1] + edge->pos[1][1]) * half;
            T diffy = y - site->p[1];
            T angle = std::atan2( diffy, x - site->p[0] );
            if( diffy < 0 ) {
                angle = angle + sm::mathconst<T>::two_pi;
            }
            return angle;
        }

        static int jcv_graphedge_eq(jcv_graphedge<T>* a, jcv_graphedge<T>* b)
        {
            return T_eq(a->angle, b->angle) && jcv_point_eq( &a->pos[0], &b->pos[0] ) && jcv_point_eq( &a->pos[1], &b->pos[1] );
        }

        static void jcv_sortedges_insert(jcv_site<T>* site, jcv_graphedge<T>* edge)
        {
            // Special case for the head end
            jcv_graphedge<T>* prev = 0;
            if (site->edges == 0 || site->edges->angle >= edge->angle)
            {
                edge->next = site->edges;
                site->edges = edge;
            }
            else
            {
                // Locate the node before the point of insertion
                jcv_graphedge<T>* current = site->edges;
                while(current->next != 0 && current->next->angle < edge->angle)
                {
                    current = current->next;
                }
                prev = current;
                edge->next = current->next;
                current->next = edge;
            }

            // check to avoid duplicates
            if (prev && jcv_graphedge_eq(prev, edge))
            {
                prev->next = edge->next;
            }
            else if (edge->next && jcv_graphedge_eq(edge, edge->next))
            {
                edge->next = edge->next->next;
            }
        }

        static void jcv_finishline(jcv_context_internal<T>* internal, edge<T>* e)
        {
            if( !edge_clipline(internal, e) ) {
                return;
            }

            // Make sure the graph edges are CCW
            int flip = jcv_determinant(&e->sites[0]->p, &e->pos[0], &e->pos[1]) > (T)0 ? 0 : 1;

            for( int i = 0; i < 2; ++i )
            {
                jcv_graphedge<T>* ge = jcv_alloc_graphedge(internal);

                ge->edge_ = e;
                ge->next = 0;
                ge->neighbor = e->sites[1-i];
                ge->pos[flip] = e->pos[i];
                ge->pos[1-flip] = e->pos[1-i];
                ge->angle = jcv_calc_sort_metric(e->sites[i], ge);

                jcv_sortedges_insert( e->sites[i], ge );
            }
        }


        static void jcv_endpos(jcv_context_internal<T>* internal, edge<T>* e, const jcv_point<T>* p, int direction)
        {
            e->pos[direction] = *p;

            if( !jcv_is_valid(&e->pos[1 - direction]) )
                return;

            jcv_finishline(internal, e);
        }

        static void jcv_create_corner_edge(jcv_context_internal<T>* internal, const jcv_site<T>* site, jcv_graphedge<T>* current, jcv_graphedge<T>* gap)
        {
            gap->neighbor   = 0;
            gap->pos[0]     = current->pos[1];

            if( current->pos[1][0] < internal->rect.max[0] && current->pos[1][1] == internal->rect.min[1] )
            {
                gap->pos[1][0] = internal->rect.max[0];
                gap->pos[1][1] = internal->rect.min[1];
            }
            else if( current->pos[1][0] > internal->rect.min[0] && current->pos[1][1] == internal->rect.max[1] )
            {
                gap->pos[1][0] = internal->rect.min[0];
                gap->pos[1][1] = internal->rect.max[1];
            }
            else if( current->pos[1][1] > internal->rect.min[1] && current->pos[1][0] == internal->rect.min[0] )
            {
                gap->pos[1][0] = internal->rect.min[0];
                gap->pos[1][1] = internal->rect.min[1];
            }
            else if( current->pos[1][1] < internal->rect.max[1] && current->pos[1][0] == internal->rect.max[0] )
            {
                gap->pos[1][0] = internal->rect.max[0];
                gap->pos[1][1] = internal->rect.max[1];
            }

            gap->angle = jcv_calc_sort_metric(site, gap);
        }

        static edge<T>* jcv_create_gap_edge(jcv_context_internal<T>* internal, jcv_site<T>* site, jcv_graphedge<T>* ge)
        {
            edge<T>* edge  = jcv_alloc_edge(internal);
            edge->pos[0]    = ge->pos[0];
            edge->pos[1]    = ge->pos[1];
            edge->sites[0]  = site;
            edge->sites[1]  = 0;
            edge->a = edge->b = edge->c = 0;
            edge->next      = internal->edges;
            internal->edges = edge;
            return edge;
        }

        static void jcv_boxshape_fillgaps(const jcv_clipper<T>* clipper, jcv_context_internal<T>* allocator, jcv_site<T>* site)
        {
            // They're sorted CCW, so if the current->pos[1] != next->pos[0], then we have a gap
            jcv_graphedge<T>* current = site->edges;
            if( !current )
            {
                // No edges, then it should be a single cell
                assert( allocator->numsites == 1 );

                jcv_graphedge<T>* gap = jcv_alloc_graphedge(allocator);
                gap->neighbor   = 0;
                gap->pos[0]     = clipper->min;
                gap->pos[1][0]   = clipper->max[0];
                gap->pos[1][1]   = clipper->min[1];
                gap->angle      = jcv_calc_sort_metric(site, gap);
                gap->next       = 0;
                gap->edge_       = jcv_create_gap_edge(allocator, site, gap);

                current = gap;
                site->edges = gap;
            }

            jcv_graphedge<T>* next = current->next;
            if( !next )
            {
                jcv_graphedge<T>* gap = jcv_alloc_graphedge(allocator);
                jcv_create_corner_edge(allocator, site, current, gap);
                gap->edge_ = jcv_create_gap_edge(allocator, site, gap);

                gap->next = current->next;
                current->next = gap;
                current = gap;
                next = site->edges;
            }

            while( current && next )
            {
                int current_edge_flags = jcv_get_edge_flags(&current->pos[1], &clipper->min, &clipper->max);
                if( current_edge_flags && !jcv_point_eq(&current->pos[1], &next->pos[0]))
                {
                    // Cases:
                    //  Current and Next on the same border
                    //  Current on one border, and Next on another border
                    //  Current on the corner, Next on the border
                    //  Current on the corner, Next on another border (another corner in between)

                    int next_edge_flags = jcv_get_edge_flags(&next->pos[0], &clipper->min, &clipper->max);
                    if (current_edge_flags & next_edge_flags)
                    {
                        // Current and Next on the same border
                        jcv_graphedge<T>* gap = jcv_alloc_graphedge(allocator);
                        gap->neighbor   = 0;
                        gap->pos[0]     = current->pos[1];
                        gap->pos[1]     = next->pos[0];
                        gap->angle      = jcv_calc_sort_metric(site, gap);
                        gap->edge_       = jcv_create_gap_edge(allocator, site, gap);

                        gap->next = current->next;
                        current->next = gap;
                    }
                    else {
                        // Current and Next on different borders
                        int corner_flag = edge_flags_to_corner(current_edge_flags);
                        if (corner_flag)
                        {
                            // we are already at one corner, so we need to find the next one
                            corner_flag = jcv_corner_rotate_90(corner_flag);
                        }
                        else
                        {
                            // we are on the middle of a border
                            // we need to find the adjacent corner, following the borders CCW
                            if      (current_edge_flags == EDGE_TOP)    { corner_flag = JCV_CORNER_TOP_LEFT; }
                            else if (current_edge_flags == EDGE_LEFT)   { corner_flag = JCV_CORNER_BOTTOM_LEFT; }
                            else if (current_edge_flags == EDGE_BOTTOM) { corner_flag = JCV_CORNER_BOTTOM_RIGHT; }
                            else if (current_edge_flags == EDGE_RIGHT)  { corner_flag = JCV_CORNER_TOP_RIGHT; }

                        }
                        jcv_point<T> corner = jcv_corner_to_point(corner_flag, &clipper->min, &clipper->max);

                        jcv_graphedge<T>* gap = jcv_alloc_graphedge(allocator);
                        gap->neighbor   = 0;
                        gap->pos[0]     = current->pos[1];
                        gap->pos[1]     = corner;
                        gap->angle      = jcv_calc_sort_metric(site, gap);
                        gap->edge_       = jcv_create_gap_edge(allocator, site, gap);

                        gap->next = current->next;
                        current->next = gap;
                    }
                }

                current = current->next;
                if( current )
                {
                    next = current->next;
                    if( !next )
                        next = site->edges;
                }
            }
        }


        // Since the algorithm leaves gaps at the borders/corner, we want to fill them
        static void jcv_fillgaps(jcv_diagram<T>* diagram)
        {
            jcv_context_internal<T>* internal = diagram->internal;
            if (!internal->clipper.fill_fn)
                return;

            for( int i = 0; i < internal->numsites; ++i )
            {
                jcv_site<T>* site = &internal->sites[i];
                internal->clipper.fill_fn(&internal->clipper, internal, site);
            }
        }


        static void jcv_circle_event(jcv_context_internal<T>* internal)
        {
            jcv_halfedge<T>* left      = (jcv_halfedge<T>*)jcv_pq_pop(internal->eventqueue);

            jcv_halfedge<T>* leftleft  = left->left;
            jcv_halfedge<T>* right     = left->right;
            jcv_halfedge<T>* rightright= right->right;
            jcv_site<T>* bottom = jcv_halfedge_leftsite(left);
            jcv_site<T>* top    = jcv_halfedge_rightsite(right);

            jcv_point<T> vertex = left->vertex;
            jcv_endpos(internal, left->edge_, &vertex, left->direction);
            jcv_endpos(internal, right->edge_, &vertex, right->direction);

            internal->last_inserted = rightright;

            jcv_pq_remove(internal->eventqueue, right);
            jcv_halfedge_unlink(left);
            jcv_halfedge_unlink(right);
            jcv_halfedge_delete(internal, left);
            jcv_halfedge_delete(internal, right);

            int direction = JCV_DIRECTION_LEFT;
            if( bottom->p[1] > top->p[1] )
            {
                jcv_site<T>* temp = bottom;
                bottom = top;
                top = temp;
                direction = JCV_DIRECTION_RIGHT;
            }

            edge<T>* edge = edge_new(internal, bottom, top);
            edge->next = internal->edges;
            internal->edges = edge;

            jcv_halfedge<T>* he = jcv_halfedge_new(internal, edge, direction);
            jcv_halfedge_link(leftleft, he);
            jcv_endpos(internal, edge, &vertex, JCV_DIRECTION_RIGHT - direction);

            jcv_point<T> p;
            if( jcv_check_circle_event( leftleft, he, &p ) )
            {
                jcv_pq_remove(internal->eventqueue, leftleft);
                leftleft->vertex    = p;
                leftleft->y         = p[1] + jcv_point_dist(&bottom->p, &p);
                jcv_pq_push(internal->eventqueue, leftleft);
            }
            if( jcv_check_circle_event( he, rightright, &p ) )
            {
                he->vertex      = p;
                he->y           = p[1] + jcv_point_dist(&bottom->p, &p);
                jcv_pq_push(internal->eventqueue, he);
            }
        }

        typedef union jcv_cast_align_struct_
        {
            char*   charp;
            void**  voidpp;
        } jcv_cast_align_struct;

        static void jcv_rect_union(jcv_rect<T>* rect, const jcv_point<T>* p)
        {
            rect->min[0] = std::min(rect->min[0], p->x());
            rect->min[1] = std::min(rect->min[1], p->y());
            rect->max[0] = std::max(rect->max[0], p->x());
            rect->max[1] = std::max(rect->max[1], p->y());
        }

        static void jcv_rect_round(jcv_rect<T>* rect)
        {
            rect->min[0] = std::floor(rect->min[0]);
            rect->min[1] = std::floor(rect->min[1]);
            rect->max[0] = std::ceil(rect->max[0]);
            rect->max[1] = std::ceil(rect->max[1]);
        }

        static void jcv_rect_inflate(jcv_rect<T>* rect, T amount)
        {
            rect->min[0] -= amount;
            rect->min[1] -= amount;
            rect->max[0] += amount;
            rect->max[1] += amount;
        }

        static int jcv_prune_duplicates(jcv_context_internal<T>* internal, jcv_rect<T>* rect)
        {
            int num_sites = internal->numsites;
            jcv_site<T>* sites = internal->sites;

            jcv_rect<T> r;
            r.min[0] = r.min[1] = std::numeric_limits<T>::max();
            r.max[0] = r.max[1] = std::numeric_limits<T>::lowest();

            int offset = 0;
            // Prune duplicates first
            for (int i = 0; i < num_sites; i++)
            {
                const jcv_site<T>* s = &sites[i];
                // Remove duplicates, to avoid anomalies
                if( i > 0 && jcv_point_eq(&s->p, &sites[i - 1].p) )
                {
                    offset++;
                    continue;
                }

                sites[i - offset] = sites[i];

                jcv_rect_union(&r, &s->p);
            }
            internal->numsites -= offset;
            if (rect) {
                *rect = r;
            }
            return offset;
        }

        static int jcv_prune_not_in_shape(jcv_context_internal<T>* internal, jcv_rect<T>* rect)
        {
            int num_sites = internal->numsites;
            jcv_site<T>* sites = internal->sites;

            jcv_rect<T> r;
            r.min[0] = r.min[1] = std::numeric_limits<T>::max();
            r.max[0] = r.max[1] = std::numeric_limits<T>::lowest();

            int offset = 0;
            for (int i = 0; i < num_sites; i++)
            {
                const jcv_site<T>* s = &sites[i];

                if (!internal->clipper.test_fn(&internal->clipper, s->p))
                {
                    offset++;
                    continue;
                }

                sites[i - offset] = sites[i];

                jcv_rect_union(&r, &s->p);
            }
            internal->numsites -= offset;
            if (rect) {
                *rect = r;
            }
            return offset;
        }

        static jcv_context_internal<T>* jcv_alloc_internal(int num_points, void* userallocctx, FJCVAllocFn allocfn, FJCVFreeFn freefn)
        {
            // Interesting limits from Euler's equation
            // Slide 81: https://courses.cs.washington.edu/courses/csep521/01au/lectures/lecture10slides.pdf
            // Page 3: https://sites.cs.ucsb.edu/~suri/cs235/Voronoi.pdf
            size_t eventssize = (size_t)(num_points*2) * sizeof(void*); // beachline can have max 2*n-5 parabolas
            size_t sitessize = (size_t)num_points * sizeof(jcv_site<T>);
            size_t memsize = sizeof(jcv_priorityqueue) + eventssize + sitessize + sizeof(jcv_context_internal<T>) + 16u; // 16 bytes padding for alignment

            char* originalmem = (char*)allocfn(userallocctx, memsize);
            memset(originalmem, 0, memsize);

            // align memory
            char* mem = (char*)jcv_align(originalmem, sizeof(void*));

            jcv_context_internal<T>* internal = (jcv_context_internal<T>*)mem;
            mem += sizeof(jcv_context_internal<T>);
            internal->mem    = originalmem;
            internal->memctx = userallocctx;
            internal->alloc  = allocfn;
            internal->free   = freefn;

            mem = (char*)jcv_align(mem, sizeof(void*));
            internal->sites = (jcv_site<T>*) mem;
            mem += sitessize;

            mem = (char*)jcv_align(mem, sizeof(void*));
            internal->eventqueue = (jcv_priorityqueue*)mem;
            mem += sizeof(jcv_priorityqueue);
            assert( ((uintptr_t)mem & (sizeof(void*)-1)) == 0 );

            jcv_cast_align_struct tmp;
            tmp.charp = mem;
            internal->eventmem = tmp.voidpp;

            assert((mem+eventssize) <= (originalmem+memsize));

            return internal;
        }

        // This version of jcv_diagram_generate allows the client to use a custom allocator
        static void jcv_diagram_generate_useralloc(int num_points, const jcv_point<T>* points, const jcv_rect<T>* rect, const jcv_clipper<T>* clipper, void* userallocctx, FJCVAllocFn allocfn, FJCVFreeFn freefn, jcv_diagram<T>* d)
        {
            if( d->internal )
                jcv_diagram_free( d );

            jcv_context_internal<T>* internal = jcv_alloc_internal(num_points, userallocctx, allocfn, freefn);

            internal->beachline_start = jcv_halfedge_new(internal, 0, 0);
            internal->beachline_end = jcv_halfedge_new(internal, 0, 0);

            internal->beachline_start->left     = 0;
            internal->beachline_start->right    = internal->beachline_end;
            internal->beachline_end->left       = internal->beachline_start;
            internal->beachline_end->right      = 0;

            internal->last_inserted = 0;

            int max_num_events = num_points*2; // beachline can have max 2*n-5 parabolas
            jcv_pq_create(internal->eventqueue, max_num_events, (void**)internal->eventmem);

            internal->numsites = num_points;
            jcv_site<T>* sites = internal->sites;

            for( int i = 0; i < num_points; ++i )
            {
                sites[i].p        = points[i];
                sites[i].edges    = 0;
                sites[i].index    = i;
            }

            qsort(sites, (size_t)num_points, sizeof(jcv_site<T>), jcv_point_cmp);

            jcv_clipper<T> box_clipper;
            if (clipper == 0) {
                // model->get_shaderprogs = &mplot::VisualBase<glver>::get_shaderprogs;
                box_clipper.test_fn = &jc::manager<T>::jcv_boxshape_test;
                box_clipper.clip_fn = &jc::manager<T>::jcv_boxshape_clip;
                box_clipper.fill_fn = &jc::manager<T>::jcv_boxshape_fillgaps;
                clipper = &box_clipper;
            }
            internal->clipper = *clipper;

            jcv_rect<T> tmp_rect;
            tmp_rect.min[0] = tmp_rect.min[1] = std::numeric_limits<T>::max();
            tmp_rect.max[0] = tmp_rect.max[1] = std::numeric_limits<T>::lowest();
            jcv_prune_duplicates(internal, &tmp_rect);

            // Prune using the test second
            if (internal->clipper.test_fn)
            {
                // e.g. used by the box clipper in the test_fn
                internal->clipper.min = rect ? rect->min : tmp_rect.min;
                internal->clipper.max = rect ? rect->max : tmp_rect.max;

                jcv_prune_not_in_shape(internal, &tmp_rect);

                // The pruning might have made the bounding box smaller
                if (!rect) {
                    // In the case of all sites being all on a horizontal or vertical line, the
                    // rect area will be zero, and the diagram generation will most likely fail
                    jcv_rect_round(&tmp_rect);
                    jcv_rect_inflate(&tmp_rect, 10);

                    internal->clipper.min = tmp_rect.min;
                    internal->clipper.max = tmp_rect.max;
                }
            }

            internal->rect = rect ? *rect : tmp_rect;

            d->min      = internal->rect.min;
            d->max      = internal->rect.max;
            d->numsites = internal->numsites;
            d->internal = internal;

            internal->bottomsite = jcv_nextsite(internal);

            jcv_priorityqueue* pq = internal->eventqueue;
            jcv_site<T>* site = jcv_nextsite(internal);

            int finished = 0;
            while( !finished )
            {
                jcv_point<T> lowest_pq_point;
                if( !jcv_pq_empty(pq) )
                {
                    jcv_halfedge<T>* he = (jcv_halfedge<T>*)jcv_pq_top(pq);
                    lowest_pq_point[0] = he->vertex[0];
                    lowest_pq_point[1] = he->y;
                }

                if( site != 0 && (jcv_pq_empty(pq) || jcv_point_less(&site->p, &lowest_pq_point) ) )
                {
                    jcv_site_event(internal, site);
                    site = jcv_nextsite(internal);
                }
                else if( !jcv_pq_empty(pq) )
                {
                    jcv_circle_event(internal);
                }
                else
                {
                    finished = 1;
                }
            }

            for( jcv_halfedge<T>* he = internal->beachline_start->right; he != internal->beachline_end; he = he->right )
            {
                jcv_finishline(internal, he->edge_);
            }

            jcv_fillgaps(d);
        }

        /**
         * Uses malloc
         * If a clipper is not supplied, a default box clipper will be used
         * If rect is null, an automatic bounding box is calculated, with an extra padding of 10 units
         * All points will be culled against the bounding rect, and all edges will be clipped against it.
         */
        static void jcv_diagram_generate( int num_points, const jcv_point<T>* points, const jcv_rect<T>* rect, const jcv_clipper<T>* clipper, jcv_diagram<T>* d )
        {
            jcv_diagram_generate_useralloc(num_points, points, rect, clipper, 0, jcv_alloc_fn, jcv_free_fn, d);
        }

        // User API
        void diagram_generate (const std::vector<jcv_point<T>>& centres)
        {
            int ncoords = static_cast<int>(centres.size());
            sm::range<T> rx, ry;
            rx.search_init();
            ry.search_init();
            for (int i = 0; i < ncoords ; ++i) {
                rx.update (centres[i][0]);
                ry.update (centres[i][1]);
            }
            // Have to actually new the diagram!
            this->diagram = new jc::jcv_diagram<T>;
            std::memset (this->diagram, 0, sizeof(jc::jcv_diagram<T>));
            this->domain = {
                jc::jcv_point<T>{rx.min - this->border_width, ry.min - this->border_width, 0.0f},
                jc::jcv_point<T>{rx.max + this->border_width, ry.max + this->border_width, 0.0f}
            };
            jc::manager<T>::jcv_diagram_generate (ncoords, centres.data(), &this->domain, 0, this->diagram);
        }

        int diagram_numsites() const
        {
            int n = 0;
            if (this->diagram) { n = this->diagram->numsites; }
            return n;
        }

        // User-configurable border width
        T border_width = std::numeric_limits<T>::epsilon();

    private:
        // Our diagram
        jc::jcv_diagram<T>* diagram = nullptr;
        // A domain for the diagram.
        jc::jcv_rect<T> domain = {};
    }; // end struct jcv

} // namespace

/*

ABOUT:

    A fast single file 2D voronoi diagram generator

(Pre mathplot) HISTORY:
    0.9     2023-01-22  - Modified the Delauney iterator creation api
    0.8     2022-12-20  - Added fix for missing border edges
                          More robust removal of duplicate graph edges
                          Added iterator for Delauney edges
    0.7     2019-10-25  - Added support for clipping against convex polygons
                        - Added EDGE_INTERSECT_THRESHOLD for edge intersections
                        - Fixed issue where the bounds calculation wasn’t considering all points
    0.6     2018-10-21  - Removed JCV_CEIL/JCV_FLOOR/JCV_FABS
                        - Optimizations: Fewer indirections, better beach head approximation
    0.5     2018-10-14  - Fixed issue where the graph edge had the wrong edge assigned (issue #28)
                        - Fixed issue where a point was falsely passing the jcv_is_valid() test (issue #22)
                        - Fixed jcv_diagram_get_edges() so it now returns _all_ edges (issue #28)
                        - Added jcv_diagram_get_next_edge() to skip zero length edges (issue #10)
                        - Added defines JCV_CEIL/JCV_FLOOR/JCV_FLT_MAX for easier configuration
    0.4     2017-06-03  - Increased the max number of events that are preallocated
    0.3     2017-04-16  - Added clipping box as input argument (Automatically calculated if needed)
                        - Input points are pruned based on bounding box
    0.2     2016-12-30  - Fixed issue of edges not being closed properly
                        - Fixed issue when having many events
                        - Fixed edge sorting
                        - Code cleanup
    0.1                 Initial version

LICENSE:

    The MIT License (MIT)

    Copyright (c) 2015-2019 Mathias Westerdahl

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.


DISCLAIMER:

    This software is supplied "AS IS" without any warranties and support

USAGE:

    The input points are pruned if

        * There are duplicates points
        * The input points are outside of the bounding box (i.e. fail the clipping test function)
        * The input points are rejected by the clipper's test function

    The input bounding box is optional (calculated automatically)

    The input domain is (-FLT_MAX, FLT_MAX] (for floats)

    The api consists of these functions:

    void jcv_diagram_generate( int num_points, const jcv_point* points, const jcv_rect* rect, const jcv_clipper* clipper, jcv_diagram* diagram );
    void jcv_diagram_generate_useralloc( int num_points, const jcv_point* points, const jcv_rect* rect, const jcv_clipper* clipper, const jcv_clipper* clipper, void* userallocctx, FJCVAllocFn allocfn, FJCVFreeFn freefn, jcv_diagram* diagram );
    void jcv_diagram_free( jcv_diagram* diagram );

    const jcv_site* jcv_diagram_get_sites( const jcv_diagram* diagram );
    const edge* jcv_diagram_get_edges( const jcv_diagram* diagram );
    const edge* jcv_diagram_get_next_edge( const edge* edge );

    An example usage:

    #define JC_VORONOI_IMPLEMENTATION
    // If you wish to use doubles
    //#define JCV_REAL_TYPE double
    #include "jc_voronoi.h"

    void draw_edges(const jcv_diagram* diagram);
    void draw_cells(const jcv_diagram* diagram);

    void generate_and_draw(int numpoints, const jcv_point* points)
    {
        jcv_diagram diagram;
        memset(&diagram, 0, sizeof(jcv_diagram));
        jcv_diagram_generate(count, points, 0, 0, &diagram);

        draw_edges(diagram);
        draw_cells(diagram);

        jcv_diagram_free( &diagram );
    }

    void draw_edges(const jcv_diagram* diagram)
    {
        // If all you need are the edges
        const edge* edge = jcv_diagram_get_edges( diagram );
        while( edge )
        {
            draw_line(edge->pos[0], edge->pos[1]);
            edge = jcv_diagram_get_next_edge(edge);
        }
    }

    void draw_cells(const jcv_diagram* diagram)
    {
        // If you want to draw triangles, or relax the diagram,
        // you can iterate over the sites and get all edges easily
        const jcv_site* sites = jcv_diagram_get_sites( diagram );
        for( int i = 0; i < diagram->numsites; ++i )
        {
            const jcv_site* site = &sites[i];

            const jcv_graphedge* e = site->edges;
            while( e )
            {
                draw_triangle( site->p, e->pos[0], e->pos[1]);
                e = e->next;
            }
        }
    }

    // Here is a simple example of how to do the relaxations of the cells
    void relax_points(const jcv_diagram* diagram, jcv_point* points)
    {
        const jcv_site* sites = jcv_diagram_get_sites(diagram);
        for( int i = 0; i < diagram->numsites; ++i )
        {
            const jcv_site* site = &sites[i];
            jcv_point sum = site->p;
            int count = 1;

            const jcv_graphedge* edge = site->edges;

            while( edge )
            {
                sum[0] += edge->pos[0][0];
                sum[1] += edge->pos[0][1];
                ++count;
                edge = edge->next;
            }

            points[site->index][0] = sum[0] / count;
            points[site->index][1] = sum[1] / count;
        }
    }

 */
