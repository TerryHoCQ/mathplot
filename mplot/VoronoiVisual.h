/*!
 * Visualize an arbitrary surface defined by values at points in 3D space (similar to
 * ScatterVisual). Using code from https://github.com/JCash/voronoi compute a 2D Voronoi
 * diagram around the data points (using their x/y values) to create the 'panels' to
 * colourize. interpolate the z values of the data points to determine the z values of
 * the edges in the Voronoi diagram. Colourize the panels based on the scalarData or
 * vectorData of the underlying VisualDataModel.
 *
 * \author Seb James
 * \date 2024
 */
#pragma once

#include <iostream>
#include <cstring>
#include <vector>
#include <array>
#include <map>
#include <set>

#include <sm/vec>
#include <sm/range>
#include <sm/quaternion>

#include <mplot/tools.h>
#include <mplot/VisualDataModel.h>

#define JC_VORONOI_IMPLEMENTATION
#include <mplot/jcvoronoi/jc_voronoi.h>

namespace mplot {

    //! The template argument F is the type of the data which this VoronoiVisual
    //! will visualize.
    template <typename F, int n_epsilons = 0, int glver = mplot::gl::version_4_1>
    class VoronoiVisual : public VisualDataModel<F, glver>
    {
        // Need a vec comparison function for a std::set of sm::vec or a std::map with a sm::vec key. See:
        // https://sebsjames.github.io/maths/ref/vec/#using-smvec-as-a-key-in-stdmap-or-within-an-stdset
        struct veccmp
        {
            bool operator()(sm::vec<float> a, sm::vec<float> b) const
            {
                return a.lexical_lessthan_beyond_epsilon(b, n_epsilons);
            }
        };

    public:
        VoronoiVisual (const sm::vec<float> _offset)
        {
            this->viewmatrix.translate (_offset);
            this->zScale.setParams (1, 0);
            this->colourScale.do_autoscale = true;
            this->colourScale2.do_autoscale = true;
            this->colourScale3.do_autoscale = true;
        }

        //! Compute 2.5D Voronoi diagram using code adapted from
        // https://github.com/JCash/voronoi. The adaptation is to add a third dimension
        // to jcv_point.
        void initializeVertices()
        {
            unsigned int ncoords = this->dataCoords == nullptr ? 0 : this->dataCoords->size();
            if (ncoords == 0) { return; }

            this->determine_datasize();
            if (this->datasize == 0) { return; }

            if (ncoords != this->datasize) {
                std::cerr << "VoronoiVisual Error: ncoords (" << ncoords << ") != datasize ("<< this->datasize
                          << "), return (no model)." << std::endl;
                return;
            }

            this->setupScaling();

            sm::quaternion<float> rq;
            if (this->data_z_direction != sm::vec<>::uz()) {
                // Find the rotation between data_z_direction and uz
                this->dcoords.resize (ncoords);
                sm::vec<float> r_axis = this->data_z_direction.cross (sm::vec<>::uz());
                r_axis.renormalize();
                float r_angle = this->data_z_direction.angle (sm::vec<>::uz(), r_axis);
                rq.rotate(r_axis, r_angle);
                for (size_t i = 0; i < ncoords; ++i) {
                    this->dcoords[i] = rq * (*this->dataCoords)[i];
                }
                this->dcoords_ptr = &this->dcoords;
            } else {
                this->dcoords_ptr = this->dataCoords;
            }

            // Use mplot::range to find the extents of dataCoords. From these create a
            // rectangle to pass to jcv_diagram_generate.
            sm::range<float> rx, ry;
            rx.search_init();
            ry.search_init();
            for (unsigned int i = 0; i < ncoords; ++i) {
                rx.update ((*this->dcoords_ptr)[i][0]);
                ry.update ((*this->dcoords_ptr)[i][1]);
            }

            // Generate the 2D Voronoi diagram
            jcv_diagram diagram;
            std::memset (&diagram, 0, sizeof(jcv_diagram));

            jcv_rect domain = {
                jcv_point{rx.min - this->border_width, ry.min - this->border_width, 0.0f},
                jcv_point{rx.max + this->border_width, ry.max + this->border_width, 0.0f}
            };
            jcv_diagram_generate (ncoords, this->dcoords_ptr->data(), &domain, 0, &diagram);

            // We obtain access the the Voronoi cell sites:
            const jcv_site* sites = jcv_diagram_get_sites (&diagram);

            // Now scan through the Voronoi cell 'sites' and 'edges' to re-assign z
            // values in the edges. This is not going to be particularly efficient, but
            // it will be fine for grids of the size that we generally visualize.
            //
            // For each site, we have a set of edges around that site. We examine each
            // edge in turn, considering the sites that cluster around each end of the
            // edge. We assign the mean z of the sites in the cluster to the edge's
            // position at that end of the edge.
            //
            // This is complicated by the fact that there may be multiple (2?) edges between pairs
            // of sites. So, need a map of edge location to z_sums

            // Mapping edge end-point locations to a container of adjacent cell centres
            std::map<sm::vec<float, 3>, std::set<sm::vec<float, 3>, veccmp>, veccmp> edge_pos_centres;
            // Mapping same edge end-point locations to the averate z value of the adjacent cell centres
            std::map<sm::vec<float, 3>, float, veccmp> edge_end_zsums;

            for (int i = 0; i < diagram.numsites; ++i) {

                // We have the current edge_1, the next edge_2 and the previous edge_0
                const jcv_site* site = &sites[i];
                jcv_graphedge* edge_first = site->edges; // The very first edge
                jcv_graphedge* edge_1 = edge_first;
                jcv_graphedge* edge_2 = edge_first;
                jcv_graphedge* edge_0 = edge_first;
                while (edge_0->next) { edge_0 = edge_0->next; }

                while (edge_1) {

                    // Set z to 0. Should be done in jcvoronoi, but haven't found out how
                    edge_1->pos[0][2] = jcv_real{0};
                    edge_1->pos[1][2] = jcv_real{0};

                    edge_2 = edge_1->next ? edge_1->next : edge_first;
                    // edge_0 already set

                    // populate the edge_pos_centres map. Known issue: In some cases, outer edges
                    // have only 1 site at 1 of their ends. This makes it a problem to
                    // compute the correct z value for the end with no site. The
                    // solution would be to modify the jcvoronoi algorithm to populate
                    // both ends of all edges with additional logic.
                    for (unsigned int j = 0; j < 2; ++j) {
                        if (edge_1->edge->sites[j]) {
                            //std::cout << "insert edge_1 " << edge_1->edge->sites[j]->p << " into edge_pos_centres[" << edge_1->pos[1] << "]\n";
                            edge_pos_centres[edge_1->pos[1]].insert (edge_1->edge->sites[j]->p);
                            //std::cout << "insert edge_1 " << edge_1->edge->sites[j]->p << " into edge_pos_centres[" << edge_1->pos[0] << "]\n";
                            edge_pos_centres[edge_1->pos[0]].insert (edge_1->edge->sites[j]->p);
                        }
                        // By definition, cellcentres_1 also gets edge_2 sites...
                        if (edge_2->edge->sites[j]) {
                            //std::cout << "insert edge_2 " << edge_2->edge->sites[j]->p << " into edge_pos_centres[" << edge_1->pos[1] << "]\n";
                            edge_pos_centres[edge_1->pos[1]].insert (edge_2->edge->sites[j]->p);
                        }
                        // and cellcentres_0 gets edge_0 sites
                        if (edge_0->edge->sites[j]) {
                            //std::cout << "insert edge_0 " << edge_0->edge->sites[j]->p << " into edge_pos_centres[" << edge_1->pos[0] << "]\n";
                            edge_pos_centres[edge_1->pos[0]].insert (edge_0->edge->sites[j]->p);
                        }
                    }

                    // Prepare for next loop
                    edge_0 = edge_1;
                    edge_1 = edge_1->next;
                }
            } // finished reassignment of z values

            // From edge_pos_centres, compute zsums for edge_end_zsums
            for (auto epc : edge_pos_centres) {
                float zsum = 0.0f;
                // cce: centres clustered-around edge
                for (auto cce : epc.second) {
                    zsum += cce[2];
                }
                if (epc.second.size() != 0) {
                    edge_end_zsums[epc.first] = zsum / epc.second.size();
                } // else do nothing
            }

            // Now go through edge_end_zsums and edges and update z values
            for (int i = 0; i < diagram.numsites; ++i) {
                const jcv_site* site = &sites[i];
                jcv_graphedge* edge_1 = site->edges; // The very first edge
                while (edge_1) {
                    // For each edge, set z from the map
                    float zsum0 = 0.0f;
                    float zsum1 = 0.0f;
                    try {
                        zsum0 = edge_end_zsums.at(edge_1->pos[0]);
                    } catch (const std::out_of_range& e) {
                        std::cout << "For edge_1: " << edge_1->pos[0] << " --- " <<  edge_1->pos[1] << std::endl;
                        std::cout << "(zsum0) no edge_end_zsums.at (" << edge_1->pos[0] << ")\n";
                    }
                    try {
                        zsum1 = edge_end_zsums.at(edge_1->pos[1]);
                    } catch (const std::out_of_range& e) {
                        std::cout << "For edge_1: " << edge_1->pos[0] << " --- " <<  edge_1->pos[1] << std::endl;                   std::cout << "(zsum1) no edge_end_zsums.at (" << edge_1->pos[1] << ")\n";
                    }
                    //std::cout << "zsum0 = " << zsum0 << " and zsum1 = " << zsum1 << std::endl;
                    edge_1->pos[0][2] = zsum0;
                    edge_1->pos[1][2] = zsum1;
                    edge_1 = edge_1->next;
                }
            }

            // To draw triangles iterate over the 'sites' and get the edges
            this->triangle_counts.resize (ncoords, 0);
            this->site_indices.resize (ncoords, 0);
            this->triangle_count_sum = 0;

            if (this->data_z_direction != sm::vec<>::uz()) {
                // inverse rotate
                sm::vec<float> t0 = {0.0f};
                sm::vec<float> t1 = {0.0f};
                sm::vec<float> t2 = {0.0f};
                sm::quaternion<float> rqinv = rq.invert();
                for (int i = 0; i < diagram.numsites; ++i) {
                    const jcv_site* site = &sites[i];
                    const jcv_graphedge* e = site->edges;
                    unsigned int site_triangles = 0;
                    while (e) {
                        // NB: There are 3 each of pos/col/norm vertices (and 3 indices) per
                        // triangle. Could be reduced in principle. For a random map, it
                        // comes out as about about 17*4 vertices per coordinate.
                        t0 = rqinv * site->p;
                        t1 = rqinv * e->pos[0];
                        t2 = rqinv * e->pos[1];
                        this->computeTriangle (t0, t1, t2, this->setColour(site->index));
                        ++site_triangles;
                        e = e->next;
                    }
                    this->triangle_counts[i] = site_triangles;
                    this->site_indices[i] = site->index;
                    this->triangle_count_sum += site_triangles;
                }
            } else {
                // No need to inverse rotate
                for (int i = 0; i < diagram.numsites; ++i) {
                    const jcv_site* site = &sites[i];
                    const jcv_graphedge* e = site->edges;
                    unsigned int site_triangles = 0;
                    while (e) {
                        this->computeTriangle (site->p, e->pos[0], e->pos[1], this->setColour(site->index));
                        ++site_triangles;
                        e = e->next;
                    }
                    this->triangle_counts[i] = site_triangles;
                    this->site_indices[i] = site->index;
                    this->triangle_count_sum += site_triangles;
                }
            }
            if (static_cast<unsigned int>(diagram.numsites) != ncoords) {
                std::cout << "WARNING: numsites != ncoords ?!?!\n";
            }

            // Draw optional objects
            if (this->debug_edges) {
                // Now scan through the edges drawing tubes for debug
                if (this->data_z_direction != sm::vec<>::uz()) {
                    // Apply rotations then compute
                    sm::vec<float> t0 = {0.0f};
                    sm::vec<float> t1 = {0.0f};
                    sm::quaternion<float> rqinv = rq.invert();
                    for (int i = 0; i < diagram.numsites; ++i) {
                        const jcv_site* site = &sites[i];
                        const jcv_graphedge* e = site->edges;
                        while (e) {
                            t0 = rqinv * (e->pos[0] * this->zoom);
                            t1 = rqinv * (e->pos[1] * this->zoom);
                            this->computeTube (t0, t1, mplot::colour::royalblue, mplot::colour::goldenrod2, this->voronoi_grid_thickness, 12);
                            e = e->next;
                        }
                    }

                } else {
                    // No rotations required
                    for (int i = 0; i < diagram.numsites; ++i) {
                        const jcv_site* site = &sites[i];
                        const jcv_graphedge* e = site->edges;
                        while (e) {
                            this->computeTube (e->pos[0] * this->zoom, e->pos[1] * this->zoom,
                                               mplot::colour::royalblue, mplot::colour::goldenrod2, this->voronoi_grid_thickness, 12);
                            e = e->next;
                        }
                    }
                }
            }

            if (this->show_voronoi2d) {
                if (this->data_z_direction != sm::vec<>::uz()) {
                    // Apply rotations then compute
                    sm::vec<float> t0 = {0.0f};
                    sm::vec<float> t1 = {0.0f};
                    sm::quaternion<float> rqinv = rq.invert();

                    for (int i = 0; i < diagram.numsites; ++i) {
                        const jcv_site* site = &sites[i];
                        const jcv_graphedge* e = site->edges;
                        while (e) {
                            t0 = rqinv * sm::vec<float>{ e->pos[0].x() * this->zoom, e->pos[0].y() * this->zoom, 0.0f };
                            t1 = rqinv * sm::vec<float>{ e->pos[1].x() * this->zoom, e->pos[1].y() * this->zoom, 0.0f };
                            this->computeTube (t0, t1, mplot::colour::black, mplot::colour::black, this->voronoi_grid_thickness, 6);
                            e = e->next;
                        }
                    }

                } else {
                    // Show the 2D Voronoi diagram's edges at z=0
                    for (int i = 0; i < diagram.numsites; ++i) {
                        const jcv_site* site = &sites[i];
                        const jcv_graphedge* e = site->edges;
                        while (e) {
                            this->computeTube ({ e->pos[0].x() * this->zoom, e->pos[0].y() * this->zoom, 0.0f },
                                               { e->pos[1].x() * this->zoom, e->pos[1].y() * this->zoom, 0.0f },
                                               mplot::colour::black, mplot::colour::black, this->voronoi_grid_thickness, 6);
                            e = e->next;
                        }
                    }
                }
            }

            if (this->debug_dataCoords) {
                // Add some spheres at the original data points for debugging
                for (unsigned int i = 0; i < ncoords; ++i) {
                    this->computeSphere ((*this->dataCoords)[i] * this->zoom, mplot::colour::black, this->dataCoord_sphere_size);
                }
            }

            // At end free the Voronoi diagram memory
            jcv_diagram_free (&diagram);
        }

        void reinitColoursScalar()
        {
            if (this->colourScale.do_autoscale == true) { this->colourScale.reset(); }
            this->dcolour.resize (this->scalarData->size());
            this->colourScale.transform (*(this->scalarData), this->dcolour);

            // Replace elements of vertexColors
            unsigned int tcounts = 0;
            for (std::size_t i = 0u; i < this->triangle_counts.size(); ++i) {
                auto c = this->cm.convert (this->dcolour[this->site_indices[i]]);
                std::size_t d_idx = tcounts * 9; // 3 floats per vtx, 3 vtxs per tri
                for (std::size_t j = 0; j < 3 * this->triangle_counts[i]; ++j) {
                    // This is ONE colour vertex. Need 3 per triangle.
                    this->vertexColors[d_idx + 3 * j]     = c[0];
                    this->vertexColors[d_idx + 3 * j + 1] = c[1];
                    this->vertexColors[d_idx + 3 * j + 2] = c[2];
                }
                tcounts += this->triangle_counts[i];
            }

            // Lastly, this call copies vertexColors (etc) into the OpenGL memory space
            this->reinit_colour_buffer();
        }

        //! Called by reinitColours when vectorData is not null (vectors are probably RGB colour)
        void reinitColoursVector()
        {
            if (this->colourScale.do_autoscale == true) { this->colourScale.reset(); }
            if (this->colourScale2.do_autoscale == true) { this->colourScale.reset(); }
            if (this->colourScale3.do_autoscale == true) { this->colourScale.reset(); }

            for (unsigned int i = 0; i < this->vectorData->size(); ++i) {
                this->dcolour[i] = (*this->vectorData)[i][0];
                this->dcolour2[i] = (*this->vectorData)[i][1];
                this->dcolour3[i] = (*this->vectorData)[i][2];
            }
            if (this->cm.getType() != mplot::ColourMapType::RGB
                && this->cm.getType() != mplot::ColourMapType::RGBMono
                && this->cm.getType() != mplot::ColourMapType::RGBGrey) {
                this->colourScale.transform (this->dcolour, this->dcolour);
                this->colourScale2.transform (this->dcolour2, this->dcolour2);
                this->colourScale3.transform (this->dcolour3, this->dcolour3);
            } // else assume dcolour/dcolour2/dcolour3 are all in range 0->1 (or 0-255) already

            // Replace elements of vertexColors
            unsigned int tcounts = 0;
            for (std::size_t i = 0u; i < this->triangle_counts.size(); ++i) {
                std::array<float, 3> c = this->setColour (this->site_indices[i]);
                std::size_t d_idx = tcounts * 9; // 3 floats per vtx, 3 vtxs per tri
                for (std::size_t j = 0; j < 3 * this->triangle_counts[i]; ++j) {
                    // This is ONE colour vertex. Need 3 per triangle.
                    this->vertexColors[d_idx + 3 * j]     = c[0];
                    this->vertexColors[d_idx + 3 * j + 1] = c[1];
                    this->vertexColors[d_idx + 3 * j + 2] = c[2];
                }
                tcounts += this->triangle_counts[i];
            }

            // Lastly, this call copies vertexColors (etc) into the OpenGL memory space
            this->reinit_colour_buffer();
        }

        void reinitColours()
        {
            if (this->vertexColors.size() < this->triangle_count_sum * 3) {
                throw std::runtime_error ("vertexColors is not big enough to reinitColours()");
            }

            // now sub-call the scalar or vector reinit colours function
            if (this->scalarData != nullptr) {
                this->reinitColoursScalar();
            } else if (this->vectorData != nullptr) {
                this->reinitColoursVector();
            } else {
                throw std::runtime_error ("No data to reinitColours()");
            }
        }

        //! Zoom in? To zoom in (make bigger) choose a value >1
        float zoom = 1.0f;

        //! If true, show 2.5D Voronoi edges
        bool debug_edges = false;
        //! If true, show 2D Voronoi edges
        bool show_voronoi2d = false;
        //! The thickness of the lines (tubes) used to draw the 2D Voronoi grid (if show_voronoi2d is true)
        float voronoi_grid_thickness = 0.01f;
        //! If true, show black spheres at dataCoord locations
        bool debug_dataCoords = false;
        //! The size of the black spheres are dataCoord locations
        float dataCoord_sphere_size = 0.008f;


        //! What direction should be considered 'z' when converting the data into a voronoi diagram?
        //! The data values will be rotated before the Voronoi pass, then rotated back.
        sm::vec<float> data_z_direction = sm::vec<>::uz();

        //! You can add a little extra to the rectangle that is auto-detected from the
        //! datacoordinate ranges. This defaults to epsilon to give the best possible
        //! surface with a rectangular grid.
        float border_width = std::numeric_limits<float>::epsilon();

        // Do we add index labels?
        bool labelIndices = false;
        sm::vec<float, 3> labelOffset = { 0.04f, 0.0f, 0.0f };
        float labelSize = 0.03f;

    protected:
        //! Compute a triangle from 3 arbitrary corners
        void computeTriangle (sm::vec<float> c1, sm::vec<float> c2, sm::vec<float> c3, const std::array<float, 3>& colr)
        {
            c1 *= this->zoom;
            c2 *= this->zoom;
            c3 *= this->zoom;
            // v is the face normal
            sm::vec<float> u1 = c1-c2;
            sm::vec<float> u2 = c2-c3;
            sm::vec<float> v = u1.cross(u2);
            v.renormalize();
            // Push corner vertices
            this->vertex_push (c1, this->vertexPositions);
            this->vertex_push (c2, this->vertexPositions);
            this->vertex_push (c3, this->vertexPositions);
            // Colours/normals
            for (unsigned int i = 0; i < 3U; ++i) {
                this->vertex_push (colr, this->vertexColors);
                this->vertex_push (v, this->vertexNormals);
            }
            this->indices.push_back (this->idx++);
            this->indices.push_back (this->idx++);
            this->indices.push_back (this->idx++);
        }

        //! Have to record the number of triangles in each cell in order to update the colours
        sm::vvec<unsigned int> triangle_counts;
        //! Record the data index for each Voronoi cell index
        sm::vvec<unsigned int> site_indices;
        unsigned int triangle_count_sum = 0;

        //! Internally owned version of dataCoords after rotation
        std::vector<sm::vec<float>> dcoords;
        //! A pointer either to dcoords or this->dataCoords
        const std::vector<sm::vec<float>>* dcoords_ptr;
    };

} // namespace mplot
