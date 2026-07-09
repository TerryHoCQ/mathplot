/*
 * TxtVisual example
 */
#include <memory>

import sm.vec;
import mplot.visual;
import mplot.txtvisual;

int main()
{
    mplot::Visual v (1024, 768, "TxtVisual example");

    auto tv = std::make_unique<mplot::TxtVisual<>> ("This is text\nand it's a VisualModel\n"
                                                    "It rotates and translates with the scene.\n"
                                                    "You can use newline characters [here]-->\n"
                                                    "in the source and these are reflected in the output.",
                                                    sm::vec<float>{1.0f, 0.0f, 0.5f}, mplot::TextFeatures (0.1f));
    tv->set_parent (v.get_id());
    tv->finalize();
    v.addVisualModel (tv);

    v.keepOpen();

    return 0;
}
