#include <iostream>

import sm.vec;
import mplot.colourmap;

int main()
{
    int rtn = 0;
    sm::vec<float, 3> rgb_in = { 1, 0.727f, 0.339f };
    sm::vec<float, 3> hsv = mplot::ColourMap<float>::rgb2hsv_vec (rgb_in);
    std::cout << "rgb: " << rgb_in << " maps to hsv: " << hsv << std::endl;
    return rtn;
}
