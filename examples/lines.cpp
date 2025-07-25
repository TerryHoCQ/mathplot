/*
 * Demonstrates the VisualModel line-drawing primitives. Make sure they're visually
 * correct.
 */

#include <sm/vec>
#include <mplot/Visual.h>
#include <mplot/VisualModel.h>

namespace mplot {

    // A test VisualModel which draws some lines
    template<int glver = mplot::gl::version_4_1>
    struct LinestestVisual : public VisualModel<glver>
    {
        LinestestVisual() : mplot::VisualModel<glver>() {}

        void initializeVertices()
        {
            float lth = 0.1f; // line thickness

            // A black horizontal line, length 1, width 0.1
            this->computeFlatLine ({0, 0, 0}, {1, 0, 0}, this->uz, mplot::colour::black, lth, 0.0f);

            // Moving up, a black horz line with rounded ends
            this->computeFlatLineRnd ({0, 0.5, 0}, {1, 0.5, 0}, this->uz, mplot::colour::black, lth, 0.0f);

            // Draw 3 lines to see if the line-joining build in to this overload of
            // VisualModel::computeFlatLine is doing the right thing
            this->computeFlatLine ({-0.5, 1-0.2, 0},        {0, 1, 0},
                                   {-0.5, 1-0.2, 0},        {1, 1, 0},
                                   this->uz,
                                   mplot::colour::black,
                                   lth);
            this->computeFlatLine ({0, 1, 0},        {1, 1, 0},
                                   {-0.5, 1-0.2, 0}, {1+0.5, 1+0.2, 0},
                                   this->uz,
                                   mplot::colour::crimson,
                                   lth);
            this->computeFlatLine ({1, 1, 0},        {1+0.5, 1+0.2, 0},
                                   {0, 1, 0},        {1+0.5, 1+0.2, 0},
                                   this->uz,
                                   mplot::colour::goldenrod1,
                                   lth);

            // Draw 3 lines with a different angle and one extra line
            sm::vec<float> p1 = {-0.5, 1.5, 0};
            sm::vec<float> p2 = {0,    2,   0};
            sm::vec<float> p3 = {1,    2,   0};
            sm::vec<float> p4 = {1.5,  2.5, 0};
            sm::vec<float> p5 = {2,    2,   0};

            this->computeFlatLine (p1,        p2,
                                   p1,        p3,
                                   this->uz, mplot::colour::black, lth);
            this->computeFlatLine (p2,        p3,
                                   p1,        p4,
                                   this->uz, mplot::colour::crimson, lth);
            this->computeFlatLine (p3,        p4,
                                   p2,        p5,
                                   this->uz, mplot::colour::goldenrod1, lth);
            this->computeFlatLine (p4,        p5,
                                   p3,        p5,
                                   this->uz, mplot::colour::dodgerblue2, lth);

            // Draw a square with computeFlatLine
            float left = 0.0f;
            float right = 1.0f;
            float bot = 3.0f;
            float top = 4.0f;
            sm::vec<float> lb = { left,  bot, 0.0f };
            sm::vec<float> lt = { left,  top, 0.0f };
            sm::vec<float> rt = { right, top, 0.0f };
            sm::vec<float> rb = { right, bot, 0.0f };
            // draw the vertical from bottom left to top left
            this->computeFlatLine (lb, lt, rb, rt, this->uz, mplot::colour::black, lth);
            // draw the horizontal from bottom left to bottom right
            this->computeFlatLine (rb, lb, rt, lt, this->uz, mplot::colour::crimson, lth);
            // complete the last right border (from bottom right to top right)
            this->computeFlatLine (rt, rb, lt, lb, this->uz, mplot::colour::goldenrod1, lth);
            // complete the last top border (from top left to top right)
            this->computeFlatLine (lt, rt, lb, rb, this->uz, mplot::colour::dodgerblue1, lth);

            // Test a straight line
            sm::vec<float> p0 = { 0, 4.5, 0 };
            p1 = { 1, 4.5, 0 };
            p2 = { 2, 4.5, 0 };
            this->computeFlatLine (p0, p1, p0, p2, this->uz, mplot::colour::black, lth);
            this->computeFlatLine (p1, p2, p0, p2, this->uz, mplot::colour::black, lth);

            // Different dirn vector (uy, not uz). This fails to work as expected.
            p0 = { 0, 5, 0 };
            p1 = { 1, 5, 1 };
            p2 = { 2, 5, 0 };
            this->computeFlatLine (p0, p1, p0, p2, this->uy, mplot::colour::black, lth);
            this->computeFlatLine (p1, p2, p0, p2, this->uy, mplot::colour::black, lth);
        }
    };

} // mplot

// Main program simply creates a Visual and places a LinestestVisual object in it.
int main()
{
    mplot::Visual v(1024, 768, "Lines");
    auto vm = std::make_unique<mplot::LinestestVisual<>>();
    v.bindmodel (vm);
    vm->finalize();
    v.addVisualModel (vm);
    v.keepOpen();
    return 0;
}
