/*
 * This is a VisualModel to place a length marker in your scene
 */

#pragma once

#include <array>

#include <sm/mathconst>
#include <sm/vec>
#include <sm/scale>
#include <sm/quaternion>

#include <mplot/VisualModel.h>

namespace mplot
{
    //! A length scale marker
    template<int glver = mplot::gl::version_4_1>
    class LengthscaleVisual : public VisualModel<glver>
    {
    public:
        LengthscaleVisual()  : mplot::VisualModel<glver>()
        {
            this->scene_to_units_scale.compute_scaling (0.0f, 1.0f);
            this->text_features.colour = this->colr;
        }
        LengthscaleVisual(const sm::vec<float, 3> _offset) : mplot::VisualModel<glver>(_offset)
        {
            this->scene_to_units_scale.compute_scaling (0.0f, 1.0f);
            this->text_features.colour = this->colr;
        }

        void initializeVertices()
        {
            // start coord is always the origin, the length is the inverse transf. of represented distance
            sm::vec<float> end_coord = this->axis * this->scene_to_units_scale.inverse_one (this->represented_distance);
            // The length scale marker is a flat line
            this->computeFlatLine (sm::vec<float>{0.0f}, end_coord,
                                   sm::vec<float>{0.0f}, end_coord, this->upaxis,
                                   this->colr, this->width);
            this->drawCaptionLabel (end_coord);
        }

        void drawCaptionLabel (sm::vec<float>& end_coord)
        {
            if (this->label.empty()) { return; }
            sm::vec<float> bar_centre = end_coord / 2.0f;
            auto lbl = this->makeVisualTextModel (this->text_features);
            mplot::TextGeometry geom = lbl->getTextGeometry (this->label);
            sm::vec<float> lblpos = { 0.0f };

            sm::quaternion<float> text_rotn; // no rotation by default

            if (this->axis == this->ux) {
                lblpos = bar_centre + sm::vec<float>{ -geom.half_width(), -this->width * 0.5f - 3.0f * geom.half_height(), 0.0f  };

            } else if (this->axis == this->uy) {
                // Rotated label
                lblpos = bar_centre + sm::vec<float>{ this->width * 0.5f + 3.0f * geom.half_height(), -geom.half_width(), 0.0f  };
                // Set rotation
                text_rotn.rotate (this->upaxis, sm::mathconst<float>::pi_over_2);

            } else {
                // write me for other orientations
                lblpos = bar_centre;
            }
            lbl->setupText (this->label, text_rotn, lblpos + this->mv_offset, this->text_features.colour);
            this->texts.push_back (std::move(lbl));
        }


        //! A width for the length scale line
        float width = 0.025f;
        //! A colour for the length scale tube (caption defaults to this colour, too)
        std::array<float, 3> colr = mplot::colour::black;
        //! The distance represented by this length scale marker
        float represented_distance = 1.0f;
        //! The scaling between scene units and the units that the length scale represents (by default a 1:1 mapping)
        sm::scale<float> scene_to_units_scale;
        //! A caption label to write next to the length scale
        std::string label = "1 unit";
        //! Font size, etc
        mplot::TextFeatures text_features;
        // A unit vector defining the axis along which this length scale will be drawn
        sm::vec<float> axis = this->ux;
        //! A unit vector defining what direction we want to look at the text from
        sm::vec<float> upaxis = this->uz;
    };

} // namespace mplot
