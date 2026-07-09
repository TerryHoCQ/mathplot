/*
 * Use cartgridvisuals to illustrate use of ColourMapType::HSV colourmap.
 */

#include <memory>
#include <string>

import sm.vec;
import mplot.visual;
import mplot.colourmap;
import mplot.cycliccolourvisual;

int main()
{
    // The main function is simple. Create a mplot::Visual, add a single SquareGridVisual and then 'keep it open'
    std::string titlestr = "Cyclic colour map";
    mplot::Visual v(1600, 1000, titlestr);
    v.backgroundBlack();

    sm::vec<float, 3> offset = { 0.0f, 0.0f, 0.0f };

    // TextFeatures is a nice way to specify font size, colour (and other things) for your addLabel() calls.
    mplot::TextFeatures tf (0.5f, mplot::colour::white);

    // HSVWHeel for Grid1
    sm::vec<float, 3> woffset = offset;
    auto hsvw_vis = std::make_unique<mplot::CyclicColourVisual<float>>(woffset);
    hsvw_vis->set_parent (v.get_id());
    hsvw_vis->setColour (mplot::colour::white);
    hsvw_vis->cm = mplot::ColourMapType::CET_C6;// CET_C6
    hsvw_vis->finalize();
    v.addVisualModel (hsvw_vis);

    v.keepOpen();
    return 0;
}
