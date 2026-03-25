module;

#include <cstdint>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>

export module mplot.triframevisual;

import sm.vec;

import mplot.tools;
import mplot.visualdatamodel;
import mplot.colourmap;

export namespace mplot
{
    /*!
     * The template argument Flt is the type of the data which this PointRowsMeshVisual
     * will visualize.
     *
     * Render a triangle made of 3 rods, with spheres at the vertices.
     *
     * \param sp The shader program
     *
     * \param _offset The offset within the mplot::Visual scene at which the model will
     * be drawn (used when rendering, not when creating the model's vertices)
     */
    template <typename Flt, std::int32_t glver = mplot::gl::version_4_1>
    class TriFrameVisual : public VisualDataModel<Flt, glver>
    {
    public:
        TriFrameVisual(const sm::vec<float, 3> _offset)
        {
            this->viewmatrix.translate (_offset);
        }

        void initializeVertices()
        {
            this->vertexPositions.clear();
            this->vertexNormals.clear();
            this->vertexColors.clear();
            this->indices.clear();
            this->idx = 0;

            std::uint32_t ncoords = this->dataCoords->size();
            std::uint32_t ndata = this->scalarData->size();

            std::vector<Flt> dcopy;
            if (ndata) {
                dcopy = *(this->scalarData);
                this->colourScale.do_autoscale = true;
                this->colourScale.transform (*this->scalarData, dcopy);
            } // else no scaling required - spheres will be one colour

            // Draw spheres
            for (std::uint32_t i = 0U; i < ncoords; ++i) {
                this->computeSphere ((*this->dataCoords)[i], this->cm.convert ((*this->scalarData)[i]), sradius);
            }
            // Draw tubes
            std::array<float, 3> clr = {0.3f,0.3f,0.3f};
            for (std::uint32_t i = 0U; i < ncoords; ++i) {
                sm::vec<float> v1 = (*this->dataCoords)[i];
                std::uint32_t e = (i < (ncoords-1) ? i+1 : 0);
                sm::vec<float> v2 = (*this->dataCoords)[e];
                sm::vec<float> _offset = this->viewmatrix.translation();
                this->computeTube (_offset + v1, _offset + v2, clr, clr, this->radius, this->tseg);
            }
        }

        //! tube radius
        float radius = 0.05f;
        //! sphere radius
        float sradius = 0.052f;
        //! sphere rings
        std::int32_t srings = 10;
        //! sphere segments
        std::int32_t sseg = 12;
        //! tube segments
        std::int32_t tseg = 12;
        //! A colour map for the spheres
        mplot::ColourMap<float> cm_sph;
    };

} // namespace mplot
