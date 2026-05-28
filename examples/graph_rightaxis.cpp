// A graph with the axis on the right
#include <memory>
#include <string>

import mplot.visual;
import mplot.graphvisual;
import mplot.unicode;

namespace uc = mplot::unicode;

int main()
{
    // Set up a mplot::Visual 'scene environment'.
    mplot::Visual v(1024, 768, "Right-axis only GraphVisual example");
    // Create a new GraphVisual with offset within the scene of 0,0,0
    auto gv = std::make_unique<mplot::GraphVisual<double>> (sm::vec<float, 3>{0.0f});
    gv->set_parent (v.get_id());
    gv->axisstyle = mplot::axisstyle::twinax; // axisstyle::twinax is like ::box, but it allows you to display a right hand ylabel2
    // Data for the x axis. A vvec is like std::vector, but with built-in maths methods
    sm::vvec<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (-0.5, 0.8, 14);

    // Set a graph up of x^3
    std::string ds1legend = uc::toUtf8 (uc::alpha) + "(x) = x" + uc::toUtf8 (uc::ss3);
    gv->setdata (x, x.pow(3), ds1legend, mplot::axisside::right);
    gv->ylabel = ""; // switch off the left label
    gv->ylabel2 = uc::toUtf8 (uc::alpha); // Put a Greek 'alpha' in ylabel2 for the right hand axis

    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene
    v.addVisualModel (gv);
    // Render the scene on the screen until user quits with 'Ctrl-q'
    v.keepOpen();
    // When v goes out of scope, gv will be deallocated
    return 0;
}
