#include <iostream>
#include <array>
import mplot.colourmap;

int main()
{
    mplot::ColourMap<float> cm (mplot::ColourMapType::Plasma);
    std::array<float, 3> c = cm.convert (0.5f);
    std::cout << "Colour is (R,G,B): (" << c[0] << "," << c[1] << "," c[2] << "\n";
}
