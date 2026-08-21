module;

/*!
 * \file Declares PlaneVisual to visualize a 2D plane in a 3D world
 */

#include <cstdint>
#include <array>

export module mplot.planevisual;

import sm.vec;
import mplot.visualmodel;

export namespace mplot
{
    // The style in which to render an impression of the plane
    enum class plane_style
    {
        rectangle, // dim1, dim2 width and height
        // rectangular_grid, // dim1, dim2 width and height, dim3 grid square side // Use a GridVisual for something this fancy
        circle,    // dim1 is radius
        ellipse    // dim1 and dim2 the two axis radii
    };

    //! A class to visualize a plane (or a plane segment/patch)
    template<std::int32_t glver = mplot::gl::version_4_1>
    class PlaneVisual : public VisualModel<glver>
    {
    public:
        PlaneVisual(const sm::vec<float> _offset)
        {
            // The offset acts as your plane offset, so we only have to define a normal for this PlaneVisual
            this->viewmatrix.translate (_offset);
        }

        //! Do the computations to initialize the vertices that will represent the Quivers.
        void initializeVertices()
        {
            if (this->style == plane_style::circle) {
                this->computeFlatPoly (sm::vec<float>{}, this->normal, this->colour, this->dim1, 24);
            }
            if (this->show_normal) {
                this->computeArrow (sm::vec<>{}, this->normal, this->norm_colour, dim1 / 100.0f);
            }
        }

        // The plane's normal
        sm::vec<float, 3> normal;

        // How to draw the plane? circle by default, rectangle or ellipse if I implement them.
        plane_style style = plane_style::circle;

        // Extents for the plane
        float dim1 = 100.0f;
        float dim2 = 100.0f;

        bool show_normal = false;

        std::array<float, 3> colour = mplot::colour::black;

        std::array<float, 3> norm_colour = mplot::colour::blue2;
    };

} // namespace mplot
