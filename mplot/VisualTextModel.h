/*!
 * \file
 *
 * Declares a class to hold vertices of the quads that are the backing for a sequence of text
 * characters. This is for use in VisualModel-derived classes. Within the backend, the
 * VisualTextModelImpl classes are used directly.
 *
 * There is a hierarchy of implementation files and a base class underlying this, but in client
 * code, you just use a VisualTextModel<glver>.
 *
 * \author Seb James
 * \date Oct 2020 - Mar 2025
 */

#pragma once

#ifdef GLAD_OPTION_GL_MX
# include <mplot/VisualTextModelImplMX.h>
#else
# include <mplot/VisualTextModelImplNoMX.h>
#endif

namespace mplot
{
    // glad_type is set in VisualOwnableNoMX.h or VisualOwnableMX.h
    template <int glver = mplot::gl::version_4_1>
    struct VisualTextModel : public mplot::VisualTextModelImpl<glver, mplot::gl::multicontext> {
        VisualTextModel(mplot::TextFeatures _tf)
            : mplot::VisualTextModelImpl<glver, mplot::gl::multicontext>::VisualTextModelImpl(_tf) {}
    };
}
