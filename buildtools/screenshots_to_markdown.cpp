#include <string>
#include <iostream>
#include <mplot/tools.h>

int main (int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " path/to/screenshots\n";
        return -1;
    }

    std::string path (argv[1]);

    std::vector<std::string> pngs;
    mplot::tools::readDirectoryTree (pngs, path);

    int count = 0;
    for (auto& png : pngs) {
        if (count % 3 == 0) { std::cout << "\n|"; }

// Each entry: ![Simulated annealing](https://github.com/sebsjames/mathplot/blob/main/examples/screenshots/Anneal_ASA.png?raw=true)  Simulated annealing and hexgrids |
        mplot::tools::stripUnixPath (png) ;
        std::string base = png;
        mplot::tools::stripFileSuffix (base);
        std::cout << " ![" << base << "](https://github.com/sebsjames/mathplot/blob/main/examples/screenshots/" << png << "?raw=true) " << base << "| ";
        ++count;
    }
    std::cout << std::endl;

    return 0;
}
