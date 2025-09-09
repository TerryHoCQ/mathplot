/*!
 * \file
 *
 * Declares a VisualModel class to hold the vertices that make up some individual model object that
 * can be part of an OpenGL scene. This is the user-facing VisualModel.
 *
 * \author Seb James
 * \date May 2019 - March 2025
 */

#pragma once

#ifdef GLAD_OPTION_GL_MX
# include <mplot/VisualModelImplMX.h>
#else
# include <mplot/VisualModelImplNoMX.h>
#endif

namespace mplot {

    /*!
     * An OpenGL model class
     *
     * This class is the 'OpenGL model' class. It has the common code to create the vertices for
     * some individual OpenGL model which is to be rendered in a 3-D scene.
     *
     * Some OpenGL models are derived directly from VisualModel; see for example mplot::CoordArrows.
     *
     * Other models in mathplot are derived via mplot::VisualDataModel, which adds a common
     * mechanism for managing the data which is to be visualised by the final 'Visual' object (such
     * as mplot::HexGridVisual or mplot::ScatterVisual)
     *
     * The base and implementation classes underlying class VisualModel contain some common 'object
     * primitives' code, such as computeSphere and computeCone, which compute the vertices that will
     * make up sphere and cone, respectively. If you need to see the primitives, look at
     * mplot/VisualModelBase.h
     *
     * Note on mplot::gl::multicontext. This is defined as a static constexpr int with the value 1
     * or 0 in <mplot/VisualOwnableNoMX.h> or <mplot/VisualOwnableMX.h>, one or other of which must have
     * been #included before you include <mplot/VisualModel.h>
     */
    template <int glver = mplot::gl::version_4_1>
    struct VisualModel : public mplot::VisualModelImpl<glver, mplot::gl::multicontext> {
        VisualModel() : mplot::VisualModelImpl<glver, mplot::gl::multicontext>::VisualModelImpl() {}
        VisualModel(const sm::vec<float, 3>& _offset)
            : mplot::VisualModelImpl<glver, mplot::gl::multicontext>::VisualModelImpl(_offset) {}
    };

} // namespace mplot
