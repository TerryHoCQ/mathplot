/*!
 * VisualModels which have data.
 */
#pragma once

#include <vector>
#include <cstdint>
#include <sm/vec>
#include <sm/vvec>
#include <sm/scale>
#include <sm/centroid>
#include <mplot/VisualModel.h>
#include <mplot/ColourMap.h>

namespace mplot
{
    //! VisualDataModel implementation base class containing common functionality - all the
    //! sm::scale objects and methods.
    template <typename T, int glver>
    struct VisualDataModel_impl_base : public VisualModel<glver>
    {
        VisualDataModel_impl_base() : mplot::VisualModel<glver>::VisualModel() {}
        VisualDataModel_impl_base (const sm::vec<float> _offset) : mplot::VisualModel<glver>::VisualModel (_offset) {}
        //! Deconstructor should *not* deallocate data - client code should do that
        ~VisualDataModel_impl_base() {}

        //! Reset the autoscaled flags so that the next time data is transformed by
        //! the Scale objects they will autoscale again (assuming they have
        //! do_autoscale set true).
        void clearAutoscale()
        {
            if (this->zScale.do_autoscale == true) { this->zScale.reset(); }
            if (this->colourScale.do_autoscale == true) { this->colourScale.reset(); }
            if (this->colourScale2.do_autoscale == true) { this->colourScale2.reset(); }
            if (this->colourScale3.do_autoscale == true) { this->colourScale3.reset(); }
            if (this->vectorScale.do_autoscale == true) { this->vectorScale.reset(); }
        }

        void clearAutoscaleZ() { if (this->zScale.do_autoscale == true) { this->zScale.reset(); } }
        void clearAutoscaleColour()
        {
            if (this->colourScale.do_autoscale == true) { this->colourScale.reset(); }
            if (this->colourScale2.do_autoscale == true) { this->colourScale2.reset(); }
            if (this->colourScale3.do_autoscale == true) { this->colourScale3.reset(); }
        }
        void clearAutoscaleVector() { if (this->vectorScale.do_autoscale == true) { this->vectorScale.reset(); } }

        void setZScale (const sm::scale<T, float>& zscale) { this->zScale = zscale; }
        void setCScale (const sm::scale<T, float>& cscale) { this->colourScale = cscale; }

        void updateZScale (const sm::scale<T, float>& zscale)
        {
            this->zScale = zscale;
            this->reinit();
        }

        void updateCScale (const sm::scale<T, float>& cscale)
        {
            this->colourScale = cscale;
            this->reinit();
        }

        void setVectorScale (const sm::scale<sm::vec<T>>& vscale)
        {
            this->vectorScale = vscale;
            this->reinit();
        }

        void setColourMap (ColourMapType _cmt, const float _hue = 0.0f)
        {
            this->cm.setHue (_hue);
            this->cm.setType (_cmt);
        }

        //! An overridable function to set the colour of rect ri
        std::array<float, 3> setColour (uint64_t ri)
        {
            std::array<float, 3> clr = { 0.0f, 0.0f, 0.0f };
            if (this->cm.numDatums() == 3) {
                if constexpr (std::is_integral<std::decay_t<T>>::value) {
                    // Differs from above as we divide by 255 to get value in range 0-1
                    clr = this->cm.convert (this->dcolour[ri]/255.0f, this->dcolour2[ri]/255.0f, this->dcolour3[ri]/255.0f);
                } else {
                    clr = this->cm.convert (this->dcolour[ri], this->dcolour2[ri], this->dcolour3[ri]);
                }
            } else if (this->cm.numDatums() == 2) {
                // Use vectorData
                clr = this->cm.convert (this->dcolour[ri], this->dcolour2[ri]);
            } else {
                clr = this->cm.convert (this->dcolour[ri]);
            }
            return clr;
        }

        //! All data models use a a colour map. Change the type/hue of this colour map
        //! object to generate different types of map.
        ColourMap<float> cm;

        //! A Scaling function for the colour map. Perhaps a scale class contains a
        //! colour map? If not, then this scale might well be autoscaled. Applied to scalarData.
        sm::scale<T, float> colourScale;
        //! Scale for second colour (when used with vectorData). This is used if the ColourMap cm is
        //! ColourMapType::DuoChrome of ColourMapType::HSV.
        sm::scale<T, float> colourScale2;
        //! scale for third colour (when used with vectorData). Use if ColourMap cm is
        //! ColourMapType::TriChrome.
        sm::scale<T, float> colourScale3;

        //! A scale to scale (or autoscale) scalarData. This might be used to set z
        //! locations of data coordinates based on scalarData. The scaling may
        sm::scale<T, float> zScale;

        //! A scaling function for the vectorData. This will scale the lengths of the
        //! vectorData.
        sm::scale<sm::vec<T>> vectorScale;

        /*
         * Scaled data. Used in GridVisual classes and PolarVisual or anywhere else where scalarData
         * or vectorData are scaled to be z values or colours.
         */

        //! A copy of the scalarData which can be transformed suitably to be the z value of the surface
        sm::vvec<float> dcopy;
        //! A copy of the scalarData (or first field of vectorData), scaled to be a colour value
        sm::vvec<float> dcolour;
        //! For the second field of vectorData
        sm::vvec<float> dcolour2;
        //! For the third field of vectorData
        sm::vvec<float> dcolour3;

        //! The length of the data structure that will be visualized. May be length of
        //! this->scalarData or of this->vectorData.
        unsigned int datasize = 0;
    };

    //! VisualDataModel implementation that deals with std::vector pointers to scalar/vector data
    template <int32_t ctype = 0, typename T = float, int glver = mplot::gl::version_4_1>
    struct VisualDataModel_impl : public VisualDataModel_impl_base<T, glver>
    {
        void setScalarData (const std::vector<T>* _data) { this->scalarData = _data; }
        void setVectorData (const std::vector<sm::vec<T>>* _vectors) { this->vectorData = _vectors; }
        void setDataCoords (std::vector<sm::vec<float>>* _coords) { this->dataCoords = _coords; }

        //! Update the scalar data
        virtual void updateData (const std::vector<T>* _data)
        {
            this->scalarData = _data;
            this->reinit();
        }

        //! Update the scalar data with an associated z-scaling
        void updateData (const std::vector<T>* _data, const sm::scale<T, float>& zscale)
        {
            this->scalarData = _data;
            this->zScale = zscale;
            this->reinit();
        }

        //! Update the scalar data, along with both the z-scaling and the colour-scaling
        void updateData (const std::vector<T>* _data, const sm::scale<T, float>& zscale, const sm::scale<T, float>& cscale)
        {
            this->scalarData = _data;
            this->zScale = zscale;
            this->colourScale = cscale;
            this->reinit();
        }

        //! Update coordinate data and scalar data along with z-scaling for scalar data
        virtual void updateData (std::vector<sm::vec<float>>* _coords, const std::vector<T>* _data,
                                 const sm::scale<T, float>& zscale)
        {
            this->dataCoords = _coords;
            this->scalarData = _data;
            this->zScale = zscale;
            this->reinit();
        }

        //! Update coordinate data and scalar data along with z- and colour-scaling for scalar data
        virtual void updateData (std::vector<sm::vec<float>>* _coords, const std::vector<T>* _data,
                                 const sm::scale<T, float>& zscale, const sm::scale<T, float>& cscale)
        {
            this->dataCoords = _coords;
            this->scalarData = _data;
            this->zScale = zscale;
            this->colourScale = cscale;
            this->reinit();
        }

        //! Update just the coordinate data
        virtual void updateCoords (std::vector<sm::vec<float>>* _coords)
        {
            this->dataCoords = _coords;
            this->reinit();
        }

        //! Update the vector data (for plotting quiver plots)
        void updateData (const std::vector<sm::vec<T>>* _vectors)
        {
            this->vectorData = _vectors;
            this->reinit();
        }

        //! Update both coordinate and vector data
        void updateData (std::vector<sm::vec<float>>* _coords, const std::vector<sm::vec<T>>* _vectors)
        {
            this->dataCoords = _coords;
            this->vectorData = _vectors;
            this->reinit();
        }

        //! Find datasize
        void determine_datasize()
        {
            this->datasize = 0;
            if (this->vectorData != nullptr && !this->vectorData->empty()) {
                this->datasize = this->vectorData->size();
            } else if (this->scalarData != nullptr && !this->scalarData->empty()) {
                this->datasize = this->scalarData->size();
            } // else datasize remains 0
        }

        // Common function for setting up the z and colour scaling
        void setupScaling()
        {
            this->dcopy.resize (this->datasize, 0);
            this->dcolour.resize (this->datasize);

            if (this->scalarData != nullptr) {
                // What do these scaling operations do to any NaNs in scalarData? They should remain
                // NaN. Then in dcopy, might want to make them 0.
                this->zScale.transform (*(this->scalarData), this->dcopy);
                this->dcopy.replace_nan_with (this->zScale.transform_one(0.0f));
                this->colourScale.transform (*(this->scalarData), this->dcolour);

            } else if (this->vectorData != nullptr) {

                this->dcolour2.resize (this->datasize);
                this->dcolour3.resize (this->datasize);
                sm::vvec<float> veclens(this->dcopy);
                for (unsigned int i = 0; i < this->datasize; ++i) {
                    veclens[i] = (*this->vectorData)[i].length();
                    this->dcolour[i] = (*this->vectorData)[i][0];
                    this->dcolour2[i] = (*this->vectorData)[i][1];
                    // Could also extract a third colour for Trichrome vs Duochrome (or for raw RGB signal)
                    this->dcolour3[i] = (*this->vectorData)[i][2];
                }
                this->zScale.transform (veclens, this->dcopy);

                // Handle case where this->cm.getType() == mplot::ColourMapType::RGB and there is
                // exactly one colour. ColourMapType::RGB (and RGBMono/Grey) assumes R/G/B data all
                // in range 0->1 ALREADY and therefore they don't need to be re-scaled with
                // this->colourScale.
                if (this->cm.getType() != mplot::ColourMapType::RGB
                    && this->cm.getType() != mplot::ColourMapType::RGBMono
                    && this->cm.getType() != mplot::ColourMapType::RGBGrey) {
                    this->colourScale.transform (this->dcolour, this->dcolour);
                    // Dual axis colour maps like Duochrome and HSV will need to use colourScale2 to
                    // transform their second colour/axis,
                    this->colourScale2.transform (this->dcolour2, this->dcolour2);
                    // Similarly for Triple axis maps
                    this->colourScale3.transform (this->dcolour3, this->dcolour3);
                } // else assume dcolour/dcolour2/dcolour3 are all in range 0->1 (or 0-255) already
            }
        }

        sm::vec<float> coordsCentroid() const { return sm::algo::centroid (*this->dataCoords); }
        //! The data to visualize. T may simply be float or double, or, if the
        //! visualization is of directional information, such as in a quiver plot,
        const std::vector<T>* scalarData = nullptr;

        //! A container for vector data to visualize. Can also be used for colour of the
        //! hexes.
        const std::vector<sm::vec<T>>* vectorData = nullptr;

        //! The coordinates at which to visualize data, if appropriate (e.g. scatter
        //! graph, quiver plot). Note fixed type of float, which is suitable for
        //! OpenGL coordinates. Not const as child code may resize or update content.
        std::vector<sm::vec<float>>* dataCoords = nullptr;
    };

#if 0 // We could add another implementation here, in which the data are provided as std::span

    //! VisualDataModel implementation that deals with std::spans to scalar/vector data
    template<typename T, int glver>
    struct VisualDataModel_impl<1, T, glver> : public VisualDataModel_impl_base<T, glver>
    {
        // span functions
        void setScalarData (std::span<T> _data) { this->scalarData = _data; }
        // ...etc

        // span attributes instead of the std::vector<>*
        std::span<T> scalarData;
        std::span<sm::vec<T>> vectorData;
        std::span<sm::vec<float>> dataCoords;
    };
#endif

    /*!
     * VisualDataModel is an optional 'data layer' for VisualModels. It is used in several of the
     * built-in VisualModels such as HexGridVisual, GridVisual and ScatterVisual. It provides a way
     * to refer to the data (such as using std::vector<> pointers) and all the scaling functions
     * that are useful for turning the data into colours and positions in the scene. It is not
     * necessary to use VisualDataModel as your base class; you can derive directly from VisualModel
     * (for an example see InstancedScatterVisual)
     */
    template <typename T, int glver = mplot::gl::version_4_1>
    struct VisualDataModel : public VisualDataModel_impl<0, T, glver> {};

} // namespace mplot
