module;

#include <cstdint>
#include <array>
#include <string>

export module mplot.boolvisual;

import sm.vec;
import mplot.visualmodel;

export namespace mplot
{
    //! This class creates the vertices for a square boolean true/false indicator. The indicator has a tubular border
    template<std::int32_t glver = mplot::gl::version_4_1>
    class BoolVisual : public VisualModel<glver>
    {
    public:
        BoolVisual() {}

        //! Initialise with offset
        BoolVisual(const sm::vec<float, 3> _offset) { this->viewmatrix.translate (_offset); }

        void computeRectangle (const float w, const std::array<float, 3> col, const sm::vec<float, 2> os = {})
        {
            // Corners of the rectangle - make sure they're clockwise in order
            sm::vec<float, 2> c1 = { w, w };
            sm::vec<float, 2> c2 = { w, -w };
            sm::vec<float, 2> c3 = { -w, -w };
            sm::vec<float, 2> c4 = { -w, w };

            c1 /= 2.0f;
            c2 /= 2.0f;
            c3 /= 2.0f;
            c4 /= 2.0f;
            c1 += os;
            c2 += os;
            c3 += os;
            c4 += os;

            this->computeFlatQuad (c1.plus_one_dim(), c2.plus_one_dim(), c3.plus_one_dim(), c4.plus_one_dim(), col);
        }

        void tubularBorder (const sm::vec<float, 4>& r_extents, // xmin xmax ymin ymax
                            const float bz, const float tubedia,
                            const std::array<float, 3>& clr)
        {
            sm::vec<float> lb = { r_extents[0], r_extents[2], bz };
            sm::vec<float> lt = { r_extents[0], r_extents[3], bz };
            sm::vec<float> rt = { r_extents[1], r_extents[3], bz };
            sm::vec<float> rb = { r_extents[1], r_extents[2], bz };

            sm::vec<float> lblt = lt - lb;
            lblt.renormalize();
            sm::vec<float> ltrt = rt - lt;
            ltrt.renormalize();
            sm::vec<float> rtrb = rb - rt;
            rtrb.renormalize();
            sm::vec<float> rblb = lb - rb;
            rblb.renormalize();

            float rad = tubedia * 0.5f;
            this->computeOpenTube (lb, lt,  -(rblb + lblt), (lblt + ltrt),  clr, clr, rad, 20);
            this->computeOpenTube (lt, rt,  -(lblt + ltrt), (ltrt + rtrb),  clr, clr, rad, 20);
            this->computeOpenTube (rt, rb,  -(ltrt + rtrb), (rtrb + rblb),  clr, clr, rad, 20);
            this->computeOpenTube (rb, lb,  -(rtrb + rblb), (rblb + lblt),  clr, clr, rad, 20);
        }

        //! Initialize vertex buffer objects and vertex array object.
        void initializeVertices()
        {
            // Draw a rectangle in the x-y plane. That's it.
            this->computeRectangle (width, (this->value ? this->tcol : this->fcol)); // 4 indices

            // Draw a border around it
            float t_rad = width / 10.0f;
            float t_rad_2 = 2.0f * t_rad;
            sm::vec<float, 4> r_extents = { -width - t_rad_2, width + t_rad_2, -width - t_rad_2, width + t_rad_2};
            this->tubularBorder (r_extents / 2.0f, 0.0f, t_rad, mplot::colour::black);
            if (!lbl.empty()) {
                // Draw label/colours. These positions could be better determined (using TextGeometry)
                float od = width / 2.0f + 2.0f * width / 10.0f; // outer dimension
                this->computeRectangle (width / 5.0f, this->fcol, sm::vec<float, 2>{+width / 4.0f,                 -od - width / 8.0f});
                this->computeRectangle (width / 5.0f, this->tcol, sm::vec<float, 2>{-width / 4.0f - width / 5.0f , -od - width / 8.0f});
                this->addLabel (this->lbl, sm::vec<float>{-od + 0.02f, od + width / 30.0f, 0}, mplot::TextFeatures(0.05f));
                this->addLabel (std::string("F"), sm::vec<float>{+width * 0.4f, -od - (0.21f * width), 0.0f}, mplot::TextFeatures(0.05f));
                this->addLabel (std::string("T"), sm::vec<float>{-width * 0.25f, -od - (0.21f * width), 0.0f}, mplot::TextFeatures(0.05f));
            }
        }

        // reinitColours to change the last rectangles worth of colour
        void reinitColours()
        {
            // Change colour of just the rectangle
            auto c = this->value ? this->tcol : this->fcol;
            for (std::uint32_t i = 0u; i < 4u; ++i) {
                this->vertexColors[3 * i] = c[0];
                this->vertexColors[3 * i + 1] = c[1];
                this->vertexColors[3 * i + 2] = c[2];
            }
            this->reinit_colour_buffer();
        }

        // Is the boolean true or false?
        bool value = false;
        // width of the square indicator
        float width = 0.3f;
        //! The colour for true
        std::array<float, 3> tcol = mplot::colour::springgreen2;
        //! The colour for false
        std::array<float, 3> fcol = mplot::colour::crimson;
        //! The label
        std::string lbl = "";
    };

} // namespace mplot
