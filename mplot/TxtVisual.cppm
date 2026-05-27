// Add some text as a VisualModel
module;

#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <stdexcept>

export module mplot.txtvisual;

import sm.vec;
import mplot.visualmodel;
import mplot.tools;
export import mplot.textfeatures;

export namespace mplot
{
    template<int glver = mplot::gl::version_4_1>
    class TxtVisual : public VisualModel<glver>
    {
    public:
        TxtVisual (const std::string& _text,
                   const sm::vec<float, 3>& _offset,
                   const mplot::TextFeatures& _tfeatures)
        {
            this->viewmatrix.translate (_offset);
            this->text = _text;
            this->tfeatures = _tfeatures;
        }

        void initializeVertices()
        {
            // No op, but add text
            this->addLabel (this->text, sm::vec<float>{}, this->tfeatures);
        }

        std::string text;
        mplot::TextFeatures tfeatures;
    };
}
