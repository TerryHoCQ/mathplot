// Test a few mplot::tools functions
#include <iostream>
#include <string>
#include <vector>

import mplot.tools;

int main()
{
    int rtn = 0;

    std::cout << "pwd: " << mplot::tools::getPwd() << std::endl;

    std::string input_str = "lkajwef7436473723$&\"'.BLAH";
    std::string for_filename = input_str;
    mplot::tools::conditionAsFilename (for_filename);

    std::cout << input_str << " conditionAsFilename: " << for_filename << std::endl;
    std::string expected_filename = "lkajwef7436473723____.BLAH";
    if (expected_filename != for_filename) { --rtn; }

    std::string path = ".";
    std::vector<std::string> svec{};
    mplot::tools::readDirectoryTree (svec, path);
    std::cout << "Found " << svec.size() << " regular files in the current directory" << std::endl;

    return rtn;
}
