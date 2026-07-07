#include <iostream>
#include <string>
#include <stdexcept>

import sm.vec;
import sm.vvec;
import mplot.loadpng;

int main()
{
    int rtn = 0;

    // Load an image
    std::string fn = "../../examples/bike256_65.png";

    sm::vvec<float> image_data;
    try {
        sm::vec<unsigned int, 2> dims = mplot::loadpng (fn, image_data);
        std::cout << "Image dims: " << dims << std::endl;
    } catch (const std::exception& e) {
        // Unexpected error
        std::cerr << "Failed to loadpng\n";
        --rtn;
    }

    fn = "examples/bad_name.png"; // known bad
    try {
        sm::vec<unsigned int, 2> dims = mplot::loadpng (fn, image_data);
        std::cout << "Image dims: " << dims << std::endl;
        // This should have failed
        --rtn;
    } catch (const std::exception& e) {
        // Expected error
        std::cout << "Error: " << e.what() << std::endl;
    }

    std::cout << "return rtn = "  << rtn << std::endl;
    return rtn;
}
