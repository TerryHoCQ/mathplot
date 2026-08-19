#include <string>
#include <vector>
#include <iostream>

import mplot.tools;

int main (int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " path/to/examples\n";
        return -1;
    }

    std::string path (argv[1]);

    std::vector<std::string> cpps;
    mplot::tools::readDirectoryTree (cpps, path);

    int count = 0;
    for (auto& cpp : cpps) {

        if (cpp.find (".cpp") == std::string::npos) { continue; }

        std::string basepath (cpp);
        mplot::tools::stripUnixFile (basepath);

        basepath += "/screenshots/";

        std::string basename = cpp;
        mplot::tools::stripUnixPath (basename);
        mplot::tools::stripFileSuffix (basename);

        std::string basecpp = cpp;
        mplot::tools::stripUnixPath (basecpp);

        std::string png = basepath + basename + ".png";

        if (mplot::tools::fileExists (png)) {
            if (count % 3 == 0) { std::cout << "\n|"; }
            std::cout << " ![" << basename
                      << "](https://github.com/sebsjames/mathplot/blob/main/examples/screenshots/"
                      << basename << ".png?raw=true) [" << basename
                      << "](https://github.com/sebsjames/mathplot/blob/main/examples/" << basecpp << ")| ";
            ++count;
        } else {
            std::cerr << "No PNG file for " << cpp << std::endl;
        }
    }
    std::cout << std::endl;

    return 0;
}
