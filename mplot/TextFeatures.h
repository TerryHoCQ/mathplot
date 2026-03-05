module;

#include <array>
#include <mplot/colour.h>

export module mplot.core:textfeatures;

import :visualfont;

export namespace mplot
{
    // A way to bundle up font size, colour, etc into a single object. Constructors chosen for max convenience.
    struct TextFeatures
    {
        TextFeatures(){};
        TextFeatures (const float _fontsize,
                      const int _fontres,
                      const bool _centre_horz,
                      const std::array<float, 3> _colour,
                      mplot::VisualFont _font)
            : fontsize(_fontsize), fontres(_fontres), centre_horz(_centre_horz), colour(_colour), font(_font) {}

        TextFeatures (const float _fontsize, const bool _centre_horz = false)
            : fontsize(_fontsize)
        {
            this->centre_horz = _centre_horz;
        }

        TextFeatures (const float _fontsize, const std::array<float, 3> _colour, const bool _centre_horz = false)
            : fontsize(_fontsize), colour(_colour)
        {
            this->centre_horz = _centre_horz;
        }

        TextFeatures (const float _fontsize, const int _fontres,
                      const std::array<float, 3> _colour = mplot::colour::black, const bool _centre_horz = false)
            : fontsize(_fontsize), fontres(_fontres), colour(_colour)
        {
            this->centre_horz = _centre_horz;
        }

        //! The size for the font
        float fontsize = 0.1f;
        //! The pixel resolution for the font textures
        int fontres = 24;
        //! If true, then centre the text string horizontally
        bool centre_horz = false;
        //! The font colour
        std::array<float, 3> colour = mplot::colour::black;
        //! The supported font to use when displaying a text string
        mplot::VisualFont font = mplot::VisualFont::DVSans;

        // Maybe also things like rotate, centre_vert, etc
    };

} // namespace
